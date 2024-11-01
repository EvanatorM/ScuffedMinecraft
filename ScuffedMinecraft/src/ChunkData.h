#pragma once

#include <cstdint>

#include "ChunkPos.h"

struct ChunkData
{
	uint16_t* data;

	ChunkData(uint16_t* data);
	~ChunkData();

	inline int GetIndex(int x, int y, int z) const;
	inline int GetIndex(ChunkPos localBlockPos) const;

	uint16_t GetBlock(ChunkPos blockPos);
	uint16_t GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, uint16_t block);
};