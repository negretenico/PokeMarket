#include "pch.h"
#include "Order.h"
#include "Quantity.h"
#include <expected>
namespace OrderingSystem {
	bool Order::isFilled() const {
		return remainingQuantity == 0;
	}
	std::expected<void, FillError> Order::fill(Model::Quantity qty) {
		if (qty > remainingQuantity) {
			return std::unexpected(FillError::Overfill);
		}
		remainingQuantity -= qty;
		return {};
	}
}