#pragma once
#include "Order.h"
#include "OrderID.h"
#include "Price.h"
#include "Side.h"
#include "Trade.h"
#include <expected>
#include <functional>
#include <map>
namespace OrderingSystem {
	enum class OrderBookError {
		ORDER_ID_NOT_FOUND
	};
	class Orderbook
	{
	private:
		std::map<Model::Price, Orders, std::greater<Model::Price>> bids;
		std::map<Model::Price, Orders, std::less<Model::Price>> asks;
		bool canMatch(Model::Side side, Model::Price price) const;
		Model::Trades matchOrder(const Order& incomingOrder);
	public:
		Orderbook() = default;
		~Orderbook() = default;
		Orderbook(const Orderbook&) = delete;
		void operator=(const Orderbook&) = delete;

		Orderbook(Orderbook&&) = delete;
		void operator=(Orderbook&&) = delete;
		Model::Trades addOrder(const Order& order);
		std::expected<void, OrderBookError> cancelOrder(Model::OrderId orderId);
	};
}
