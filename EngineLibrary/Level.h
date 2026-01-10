#pragma once
#include "Price.h"
#include "Quantity.h"
#include <vector>
namespace Model {
	struct Level {
		Price price;
		Quantity quantity;
	};
	using Levels = std::vector<Level>;
}