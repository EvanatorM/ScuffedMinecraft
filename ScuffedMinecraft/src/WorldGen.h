#pragma once

#include <vector>

namespace WorldGen
{
	void GenerateChunkData(int chunkX, int chunkY, int chunkZ, int chunkSize, std::vector<unsigned int>* chunkData);
}