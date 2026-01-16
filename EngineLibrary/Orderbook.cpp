#include "pch.h"
#include "Order.h"
#include "OrderID.h"
#include "Orderbook.h"
#include "Price.h"
#include "Quantity.h"
#include "Side.h"
#include "Trade.h"
#include <algorithm>
#include <expected>
#include <list>
#include <memory>
#include <vector>
using namespace Model;
namespace OrderingSystem {
	bool Orderbook::canMatch(Side side, Price price) const {

		if (side == Side::Buy) {
			if (asks.empty()) {
				return false;
			}
			const auto& [sellingPrice, _] = *asks.begin(); // we do not want accidnetal copying
			return price >= sellingPrice;
		}
		if (bids.empty()) {
			return false;
		}
		const auto& [buyingPrice, _] = *bids.begin();
		return price <= buyingPrice;
	}
	Trades Orderbook::matchOrder(const Order& incomingOrder) {
		// Placeholder implementation
		Trades trades;
		trades.reserve(asks.size() + bids.size()); //pessimistically reserve space for all orders
		while (!bids.empty() && !asks.empty()) {
			auto bid_it = bids.begin();
			auto ask_it = asks.begin();

			const auto buyingPrice = bid_it->first;
			const auto sellingPrice = ask_it->first;

			if (buyingPrice < sellingPrice) {
				break;
			}

			while (!bid_it->second.empty() && !ask_it->second.empty()) {
				auto& buyingOrders = bid_it->second;
				auto& sellingOrders = ask_it->second;
				const auto& buyOrder = buyingOrders.front();
				if (!buyOrder) {
					buyingOrders.pop_front();
					continue;
				}

				const auto& sellOrder = sellingOrders.front();
				if (!sellOrder) {
					sellingOrders.pop_front();
					continue;
				}
				auto buyId = buyOrder->getOrderId();
				auto sellId = sellOrder->getOrderId();
				Quantity tradeQty = std::min(buyOrder->getRemainingQuantity(), sellOrder->getRemainingQuantity());
				// fill both with the remainder 
				buyOrder->fill(tradeQty);
				sellOrder->fill(tradeQty);
				// if eitehr becomes filled becasue of this remove it from the queue
				if (buyOrder->isFilled()) {
					buyingOrders.pop_front();
					orders.erase(buyId);
				}
				if (sellOrder->isFilled()) {
					sellingOrders.pop_front();
					orders.erase(sellId);
				}

				//add our orders to the trade list
				trades.push_back(Trade{ buyId, buyingPrice,tradeQty });
				trades.push_back(Trade{ sellId, sellingPrice ,tradeQty });
			}
			// if either beconmes empty remove that rpice from the order book 
			if (bid_it->second.empty()) {
				bids.erase(bid_it);
			}
			if (ask_it->second.empty()) {
				asks.erase(ask_it);
			}
		}
		return trades;
	}

	Trades Orderbook::addOrder(const Order& order) {
		if (!canMatch(order.getSide(), order.getPrice())) {
			// No matches possible, add to order book directly
			auto orderPtr = std::make_shared<Order>(order);
			orders[order.getOrderId()] = orderPtr;
			if (order.getSide() == Side::Buy) {
				bids[order.getPrice()].push_back(orderPtr);
				return {};
			}
			asks[order.getPrice()].push_back(orderPtr);
			return { }; // empty trades
		}
		auto orderPtr = std::make_shared<Order>(order);
		orders[order.getOrderId()] = orderPtr;
		if (order.getSide() == Side::Buy) {
			bids[order.getPrice()].push_back(orderPtr);
		}
		asks[order.getPrice()].push_back(orderPtr);
		Trades trades = matchOrder(order);
		if (order.getRemainingQuantity() == Quantity{ 0 }) {
			return trades; // order fully filled during matching
		}
		return trades;
	}
	std::expected<void, OrderBookError> Orderbook::cancelOrder(OrderId orderId) {
		auto it = orders.find(orderId);
		if (it == orders.end()) {
			return std::unexpected(OrderBookError::ORDER_ID_NOT_FOUND);
		}
		const auto& orderPtr = it->second;
		if (orderPtr->getSide() == Side::Buy) {
			auto it = bids.find(orderPtr->getPrice());
			if (it == bids.end()) {
				return std::unexpected(OrderBookError::PRICE_NOT_FOUND);
			}
			auto& [_, orderList] = *it;
			std::erase_if(orderList, [orderId](const OrderPtr& o) { return o->getOrderId() == orderId; });
			if (orderList.empty()) {
				bids.erase(it);
			}
			return {};
		}
		auto ask_it = asks.find(orderPtr->getPrice());
		if (ask_it == asks.end()) {
			return std::unexpected(OrderBookError::PRICE_NOT_FOUND);
		}
		auto& [_, orderList] = *ask_it;
		std::erase_if(orderList, [orderId](const OrderPtr& o) { return o->getOrderId() == orderId; });
		if (orderList.empty()) {
			asks.erase(ask_it);
		}
		return {};
	}
}