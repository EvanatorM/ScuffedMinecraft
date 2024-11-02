#pragma once

#include <vector>
#include <array>

#include "Block.h"

namespace Blocks
{
	const std::vector<Block> blocks{
		Block(0, 0, 0, 0, Block::TRANSPARENT, "Air"),				// Air block
		Block(0, 0, 1, 1, Block::SOLID, "Dirt"),					// Dirt block

		Block(1, 1, 2, 2,											// Grass block
			  0, 0, 1, 1,
			  1, 0, 2, 1, Block::SOLID, "Grass Block"),

		Block(0, 1, 1, 2, Block::SOLID, "Stone"),					// Stone block

		Block(2, 1, 3, 2,											// Log
			  2, 1, 3, 2,
			  2, 0, 3, 1, Block::SOLID, "Log"),

		Block(0, 2, 1, 3, Block::LEAVES, "Leaves"),					// Leaves
		Block(1, 2, 2, 3, Block::BILLBOARD, "Grass"),				// Grass
		Block(3, 0, 4, 1, Block::BILLBOARD, "Tall Grass Bottom"),	// Tall Grass Bottom
		Block(3, 1, 4, 2, Block::BILLBOARD, "Tall Grass Top"),		// Tall Grass Top
		Block(0, 3, 1, 4, Block::BILLBOARD, "Poppy"),				// Poppy
		Block(2, 2, 3, 3, Block::BILLBOARD, "White Tulip"),			// White Tulip
		Block(3, 2, 4, 3, Block::BILLBOARD, "Pink Tulip"),			// Pink Tulip
		Block(1, 3, 2, 4, Block::BILLBOARD, "Orange Tulip"),		// Orange Tulip
		Block(0, 4, 1, 5, Block::LIQUID, "Water"),					// Water
		Block(4, 0, 5, 1, Block::SOLID, "Sand"),					// Sand
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
		WATER = 13,
		SAND = 14,
	};
}