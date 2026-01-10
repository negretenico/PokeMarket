#include "pch.h"
#include "Order.h"
#include "OrderID.h"
#include "Orderbook.h"
#include "Price.h"
#include "Side.h"
#include "Trade.h"
#include <expected>
using namespace Model;
namespace OrderingSystem {
	Trades Orderbook::addOrder(const Order& order) {
		// Placeholder implementation
		return Trades{};
	}
	std::expected<void, OrderBookError> Orderbook::cancelOrder(OrderId orderId) {
		// Placeholder implementation
		return std::unexpected(OrderBookError::ORDER_ID_NOT_FOUND);
	}
	bool Orderbook::canMatch(Side side, Price price) const {
		if (side == Side::Buy) {
			const auto& [sellingPrice, _] = *asks.begin(); // we do not want accidnetal copying
			return price >= sellingPrice;
		}
		const auto& [buyingPrice, _] = *bids.begin();
		return price <= buyingPrice;
	}
	Trades Orderbook::matchOrder(const Order& incomingOrder) {
		// Placeholder implementation
		Trades trades;
		trades.reserve(asks.size() + bids.size()); //pessimistically reserve space for all orders
		while (!bids.empty() || !asks.empty()) {
			// Check to make sure any matches can happen
			// is the current Selling Price to high for anyone trying to buy (asking price > buying price)
			// get the the current head of each queue 
			// fill both with the remainder 
			// if eitehr becomes filled becasue of this remove it from the queue
			// if either beconmes empty remove that rpice from the order book 
		}
		return trades;
	}
}