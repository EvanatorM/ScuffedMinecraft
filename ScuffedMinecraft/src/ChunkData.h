#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct ChunkData
{
	uint16_t* data;

	ChunkData(uint16_t* data);
	~ChunkData();

	inline int GetIndex(int x, int y, int z) const;
	inline int GetIndex(const glm::ivec3& localBlockPos) const;

	uint16_t GetBlock(const glm::ivec3& blockPos);
	uint16_t GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, uint16_t block);
};