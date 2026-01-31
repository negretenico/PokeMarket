#pragma once
#include "Order.h"
#include "OrderID.h"
#include "Price.h"
#include "Side.h"
#include "Trade.h"
#include <expected>
#include <functional>
#include <map>
#include <unordered_map>

namespace OrderingSystem {
	enum class OrderBookError {
		ORDER_ID_NOT_FOUND,
		PRICE_NOT_FOUND
	};
	class Orderbook
	{
	private:
		std::map<Model::Price, Orders, std::greater<Model::Price>> bids;
		std::map<Model::Price, Orders, std::less<Model::Price>> asks;
		Model::Trades possibleTrades;
		std::unordered_map<Model::OrderId, OrderPtr> orders;
		bool canMatch(Model::Side side, Model::Price price) const;
		Model::Trades matchOrder(const Order& incomingOrder);
	public:
		Orderbook() {
			possibleTrades.reserve(1'000); // preallocate space for trades to avoid dynamic allocations
		};
		~Orderbook() = default;
		Orderbook(const Orderbook&) = delete;
		void operator=(const Orderbook&) = delete;

		Orderbook(Orderbook&&) = delete;
		void operator=(Orderbook&&) = delete;
		Model::Trades addOrder(const Order& order);
		std::expected<void, OrderBookError> cancelOrder(Model::OrderId orderId);
	};
}
