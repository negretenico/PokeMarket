#pragma once
#include "OrderID.h"
#include "OrderType.h"
#include "Price.h"
#include "Quantity.h"
#include "Side.h"
#include <expected>
#include <list>
#include <memory>

namespace OrderingSystem {
	enum class FillError {
		Overfill
	};

	class Order {
	public:
		bool isFilled() const;
		std::expected<void, FillError> fill(Model::Quantity qty);
		Model::Side getSide() const { return side; }
		Model::OrderId getOrderId() const { return orderId; }
		Model::Price getPrice() const { return price; }
		Model::Quantity getRemainingQuantity() const { return remainingQuantity; }
		Model::OrderType getOrderType() const { return orderType; }
		class Builder {
		public:
			Builder() = default;
			Builder& withOrderType(Model::OrderType t) { m_orderType = t; return *this; }
			Builder& withSide(Model::Side s) { m_side = s; return *this; }
			Builder& withInitialQuantity(Model::Quantity q) { m_initialQuantity = q; return *this; }
			Builder& withPrice(Model::Price p) { m_price = p; return *this; }
			Builder& withOrderId(Model::OrderId id) { m_orderId = id; return *this; }

			Order build() const {
				Order o;
				o.orderType = m_orderType;
				o.side = m_side;
				o.initalQuantity = m_initialQuantity;
				o.remainingQuantity = m_initialQuantity;
				o.price = m_price;
				o.orderId = m_orderId;
				return o;
			}

		private:
			Model::OrderType m_orderType{};
			Model::Side m_side{};
			Model::Quantity m_initialQuantity{ 0 };
			Model::Price m_price{ 0 };
			Model::OrderId m_orderId{ 0 };
		};

	private:
		Order() = default;
		friend class Builder;
		Model::OrderType orderType;
		Model::Side side;
		Model::Quantity initalQuantity;
		Model::Quantity remainingQuantity;
		Model::Price price;
		Model::OrderId orderId;
	};
	using OrderPtr = std::shared_ptr<Order>;
	using Orders = std::list<OrderPtr>;
}