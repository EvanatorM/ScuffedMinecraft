#pragma once

#include <vector>

#include "Block.h"

namespace Blocks
{
	const std::vector<Block> blocks{
		Block(0, 0, 0, 0, true),			// Air block
		Block(0, 0, 1, 1),					// Dirt block

		Block(1, 1, 2, 2,					// Grass block
			  0, 0, 1, 1,
			  1, 0, 2, 1),

		Block(0, 1, 1, 2),					// Stone block

		Block(2, 1, 3, 2,					// Log
			  2, 1, 3, 2,
			  2, 0, 3, 1),

		Block(0, 2, 1, 3, true),			// Leaves
		Block(1, 2, 2, 3, true, true),		// Grass
		Block(3, 0, 4, 1, true, true),		// Tall Grass Bottom
		Block(3, 1, 4, 2, true, true),		// Tall Grass Top
		Block(0, 3, 1, 4, true, true),		// Poppy
		Block(2, 2, 3, 3, true, true),		// White Tulip
		Block(3, 2, 4, 3, true, true),		// Pink Tulip
		Block(1, 3, 2, 4, true, true),		// Orange Tulip
	};

	enum BLOCKS
	{
		AIR = 0,
		DIRT_BLOCK = 1,
		GRASS_BLOCK = 2,
		STONE_BLOCK = 3,
		LOG = 4,
		LEAVES = 5,
		GRASS = 6,
		TALL_GRASS_BOTTOM = 7,
		TALL_GRASS_TOP = 8,
		POPPY = 9,
		WHITE_TULIP = 10,
		PINK_TULIP = 11,
		ORANGE_TULIP = 12,
	};
}