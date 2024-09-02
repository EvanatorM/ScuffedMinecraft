#include "WorldGen.h"

#include <OpenSimplexNoise.hh>

#include "Blocks.h"

void WorldGen::GenerateChunkData(int chunkX, int chunkY, int chunkZ, int chunkSize, std::vector<unsigned int>* chunkData)
{
	chunkData->reserve(chunkSize * chunkSize * chunkSize);

	OSN::Noise<2> surfaceNoise;
	OSN::Noise<3> caveNoise;
	int startX = chunkX * chunkSize;
	int startY = chunkY * chunkSize;
	int startZ = chunkZ * chunkSize;

	for (int x = 0; x < chunkSize; x++)
	{
		for (int z = 0; z < chunkSize; z++)
		{
			int noiseY = (surfaceNoise.eval((float)(x + startX) * .1f, (float)(z + startZ) * .1f) * 3.0f) + 20;
			for (int y = 0; y < chunkSize; y++)
			{
				//chunkData.push_back(0);                              // Empty
				//chunkData.push_back(1);                              // Full
				//chunkData.push_back(((x + y + z) % 2 == 0) ? 1 : 0); // Checkerboard
				/*chunkData.push_back(                                   // Edges
					(x == 0 || x == chunkSize - 1) ||
					(y == 0 || y == chunkSize - 1) ||
					(z == 0 || z == chunkSize - 1)
					? 1 : 0
				);*/

				float noiseCaves = caveNoise.eval(
					(float)(x + startX) * .1f,
					(float)(y + startY) * .1f,
					(float)(z + startZ) * .1f);

				// Sky and Caves
				if (y + startY > noiseY)
					chunkData->push_back(0);
				else if (noiseCaves > .5f)
					chunkData->push_back(0);
				// Ground
				else if (y + startY == noiseY)
					chunkData->push_back(Blocks::GRASS_BLOCK);
				else if (y + startY > 10)
					chunkData->push_back(Blocks::DIRT_BLOCK);
				else
					chunkData->push_back(Blocks::STONE_BLOCK);
			}
		}
	}
}