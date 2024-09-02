#pragma once

#include <vector>

#include "Block.h"

namespace Blocks
{
	const std::vector<Block> blocks{
		Block(0, 0, 0, 0), // Air block
		Block(0, 0, 1, 1), // Dirt block

		Block(1, 1, 2, 2,  // Grass block
			  0, 0, 1, 1,
			  1, 0, 2, 1),

		Block(0, 1, 1, 2)  // Stone block
	};

	enum BLOCKS
	{
		AIR = 0,
		DIRT_BLOCK = 1,
		GRASS_BLOCK = 2,
		STONE_BLOCK = 3
	};
}