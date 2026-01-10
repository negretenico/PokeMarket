#pragma once
#include "Level.h"
namespace Model {
	class LevelInventory {
	public:
		LevelInventory(const Levels& bidLevels, const Levels& askLevels)
			: bids(bidLevels), asks(askLevels) {
		}
		const Levels& getBids() const {
			return bids;
		}
		const Levels& getAsks() const {
			return asks;
		}
	private:
		Levels bids;
		Levels asks;
	};
}