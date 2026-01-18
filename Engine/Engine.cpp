#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

// Include your orderbook headers
#include <string.h>
// #include "Order.h// #include "OrderID.h"       // etc.

struct ParsedOrder {
	uint64_t orderId;
	uint8_t side;
	uint64_t price;
	uint32_t quantity;
};

int main(int argc, char** argv) {
	const char* path = (argc > 1) ? argv[1] : "orders.bin";
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs) {
		std::cerr << "Failed to open " << path << '\n';
		return 1;
	}

	uint64_t totalRecords = 0;
	uint64_t totalBytes = 0;

	auto start = std::chrono::steady_clock::now();

	while (true) {
		uint8_t msgType;
		uint16_t length;
		if (!ifs.read(reinterpret_cast<char*>(&msgType), sizeof(msgType))) break;
		if (!ifs.read(reinterpret_cast<char*>(&length), sizeof(length))) break;

		// length is little-endian on x86; if running on big-endian platform, convert.
		std::vector<char> payload(length);
		if (!ifs.read(payload.data(), length)) break;

		totalBytes += sizeof(msgType) + sizeof(length) + length;

		if (msgType != 1) {
			// unknown message type - skip or handle
			continue;
		}

		// Unpack payload: <Q B Q I  (little-endian)
		ParsedOrder p;
		size_t offset = 0;
		// orderId (uint64)
		std::memcpy(&p.orderId, payload.data() + offset, sizeof(p.orderId));
		offset += sizeof(p.orderId);
		// side (uint8)
		std::memcpy(&p.side, payload.data() + offset, sizeof(p.side));
		offset += sizeof(p.side);
		// price (uint64)
		std::memcpy(&p.price, payload.data() + offset, sizeof(p.price));
		offset += sizeof(p.price);
		// quantity (uint32)
		std::memcpy(&p.quantity, payload.data() + offset, sizeof(p.quantity));
		offset += sizeof(p.quantity);

		++totalRecords;

		// TODO: construct your Model::Order object from ParsedOrder and push into your single-threaded orderbook
		// Example (adjust to your constructors/types):
		// Model::OrderId oid{p.orderId};
		// Model::Side side = (p.side == 1) ? Model::Side::Buy : Model::Side::Sell;
		// Model::Price price{p.price};
		// Model::Quantity qty{p.quantity};
		// Model::Order order{ oid, side, price, qty };
		// static OrderingSystem::Orderbook book;
		// book.addOrder(order);

		// Minimal progress print every N records to avoid slowing the run
		if ((totalRecords & 0xFFFF) == 0) {
			std::cout << "\rParsed records: " << totalRecords << " bytes: " << totalBytes << std::flush;
		}
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> secs = end - start;
	std::cout << "\nDone. Records=" << totalRecords << " bytes=" << totalBytes
		<< " time=" << secs.count() << "s throughput=" << (totalRecords / secs.count()) << " rec/s\n";

	return 0;
}