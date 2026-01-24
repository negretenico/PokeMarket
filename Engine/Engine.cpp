#include "../EngineLibrary/Order.h"
#include "../EngineLibrary/OrderType.h"
#include "../EngineLibrary/Orderbook.h"
#include "../EngineLibrary/Side.h"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <string.h>
#include <vector>
#include <windows.h>
using namespace OrderingSystem;
using namespace Model;
struct ParsedOrder {
	uint64_t orderId;
	uint8_t side;
	uint64_t price;
	uint32_t quantity;
};


void setCPUAffinity(bool isEnabled) {
	if (!isEnabled) {
		return;
	}
	DWORD_PTR mask = 1;  // Binary: 0001 (core 0)
	if (!SetProcessAffinityMask(GetCurrentProcess(), mask)) {
		std::cerr << "Warning: failed to set CPU affinity" << '\n';
		return;
	}
	std::cout << "Pinning to CPU core 0" << '\n';
}

int main(int argc, char** argv) {
	// Parse command-line: first non-flag arg is path; -cpu_affinity [true|false|1|0]
	std::string path = "orders.bin";
	bool cpuAffinity = false;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-cpu_affinity") {
			if (i + 1 < argc) {
				std::string val = argv[++i];
				if (val == "1" || val == "true" || val == "True" || val == "TRUE") {
					cpuAffinity = true;
				}
				continue;
			}
			cpuAffinity = true;
		}
		else if (!arg.empty() && arg[0] != '-' && path == "orders.bin") {
			path = arg;
		}
	}

	std::ifstream ifs(path.c_str(), std::ios::binary);
	if (!ifs) {
		std::cerr << "Failed to open " << path << '\n';
		return 1;
	}

	setCPUAffinity(cpuAffinity);
	uint64_t totalRecords = 0;
	uint64_t totalBytes = 0;

	auto start = std::chrono::steady_clock::now();
	auto deadline = start + std::chrono::minutes(10); // 10 minute timeout
	Orderbook orderbook;
	while (true) {
		if (std::chrono::steady_clock::now() >= deadline) {
			std::cerr << "Timeout reached after 10 minutes\n";
			break;
		}
		uint8_t msgType;
		uint16_t length;
		if (!ifs.read(reinterpret_cast<char*>(&msgType), sizeof(msgType))) break;
		if (!ifs.read(reinterpret_cast<char*>(&length), sizeof(length))) break;

		// length is little-endian on x86; if running on big-endian platform, convert.
		std::vector<char> payload(length);
		if (!ifs.read(payload.data(), length)) break;

		totalBytes += sizeof(msgType) + sizeof(length) + length;

		// After parsing the header, before msgType check:
		if (msgType == 1) {
			// NEW ORDER
			ParsedOrder p;
			size_t offset = 0;
			std::memcpy(&p.orderId, payload.data() + offset, sizeof(p.orderId));
			offset += sizeof(p.orderId);
			std::memcpy(&p.side, payload.data() + offset, sizeof(p.side));
			offset += sizeof(p.side);
			std::memcpy(&p.price, payload.data() + offset, sizeof(p.price));
			offset += sizeof(p.price);
			std::memcpy(&p.quantity, payload.data() + offset, sizeof(p.quantity));

			const auto order = OrderingSystem::Order::Builder()
				.withOrderId(p.orderId)
				.withPrice(p.price)
				.withInitialQuantity(p.quantity)
				.withSide((p.side == 1) ? Side::Buy : Side::Sell)
				.withOrderType(OrderType::GoodForDay)
				.build();
			orderbook.addOrder(order);

		}
		else if (msgType == 2) {
			// CANCEL ORDER
			uint64_t cancelOrderId;
			std::memcpy(&cancelOrderId, payload.data(), sizeof(cancelOrderId));
			orderbook.cancelOrder(cancelOrderId);

		}
		else {
			// Unknown message type - skip
			continue;
		}

		++totalRecords;
		// Minimal progress print every N records to avoid slowing the run
		if ((totalRecords & 0xFFFF) == 0) {
			std::cout << "\rParsed records: " << totalRecords << " bytes: " << totalBytes << std::flush << '\n';
		}
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> secs = end - start;
	std::cout << "\nDone. Records=" << totalRecords << " bytes=" << totalBytes
		<< " time=" << secs.count() << "s throughput=" << (totalRecords / secs.count()) << " rec/s\n";
	std::cin.get();
	return 0;
}