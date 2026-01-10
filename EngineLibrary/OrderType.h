#pragma once
namespace Model {
	enum class OrderType {
		GoodTillCancel,
		FillAndKill,
		FillOrKill,
		GoodForDay,
	};
}