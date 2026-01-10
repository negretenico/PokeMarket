#pragma once
#include "OrderId.h"
#include "Price.h"
#include "Quantity.h"
#include <vector>
namespace Model {
	struct Trade {
		OrderId orderId;
		Price price;
		Quantity quantity;
	};
	using Trades = std::vector<Trade>;
	class TradeInventory {
	public:
		TradeInventory(const Trades& bidTrades, const Trades& askTrades)
			: bids(bidTrades), asks(askTrades) {
		}
		const Trades& getAsks() const {
			return asks;
		}
		const Trades& getBids() const {
			return bids;
		}
	private:
		Trades asks;
		Trades bids;
	};
}
