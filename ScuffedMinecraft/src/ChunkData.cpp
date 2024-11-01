#include "ChunkData.h"

#include "Planet.h"

ChunkData::ChunkData(uint16_t* data)
	: data(data)
{
	
}

ChunkData::~ChunkData()
{
	delete[] data;
}

inline int ChunkData::GetIndex(int x, int y, int z) const
{
	return x * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + y;
}

inline int ChunkData::GetIndex(ChunkPos localBlockPos) const
{
	return localBlockPos.x * CHUNK_SIZE * CHUNK_SIZE + localBlockPos.z * CHUNK_SIZE + localBlockPos.y;
}

uint16_t ChunkData::GetBlock(ChunkPos blockPos)
{
	return data[GetIndex(blockPos)];
}

uint16_t ChunkData::GetBlock(int x, int y, int z)
{
	return data[GetIndex(x, y, z)];
}

void ChunkData::SetBlock(int x, int y, int z, uint16_t block)
{
	data[GetIndex(x, y, z)] = block;
}