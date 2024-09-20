#include "WorldGen.h"

#include <algorithm>
#include <OpenSimplexNoise.hh>

#include "Blocks.h"

void WorldGen::GenerateChunkData(int chunkX, int chunkY, int chunkZ, int chunkSize, std::vector<unsigned int>* chunkData)
{
	// Init noise
	OSN::Noise<2> noise2D;
	OSN::Noise<3> noise3D;

	// Init noise settings
	NoiseSettings surfaceSettings[] {
		{ 0.01f, 20.0f, 0 },
		{ 0.1f,   3.0f, 0 }
	};
	int surfaceSettingsLength = sizeof(surfaceSettings) / sizeof(*surfaceSettings);

	NoiseSettings caveSettings[] {
		{ 0.05f, 1.0f, 0, .5f, 0, 100 }
	};
	int caveSettingsLength = sizeof(caveSettings) / sizeof(*caveSettings);

	NoiseSettings oreSettings[] {
		{ 0.075f, 1.0f, 8.54f, .75f, 1, 0 }
	};
	int oreSettingsLength = sizeof(oreSettings) / sizeof(*oreSettings);

	// Set vector size
	chunkData->reserve(chunkSize * chunkSize * chunkSize);

	// Account for chunk position
	int startX = chunkX * chunkSize;
	int startY = chunkY * chunkSize;
	int startZ = chunkZ * chunkSize;

	for (int x = 0; x < chunkSize; x++)
	{
		for (int z = 0; z < chunkSize; z++)
		{
			// Surface noise
			int noiseY = 20;
			for (int i = 0; i < surfaceSettingsLength; i++)
			{
				noiseY += noise2D.eval(
					(float)((x + startX) * surfaceSettings[i].frequency) + surfaceSettings[i].offset,
					(float)((z + startZ) * surfaceSettings[i].frequency) + surfaceSettings[i].offset)
					* surfaceSettings[i].amplitude;
			}

			for (int y = 0; y < chunkSize; y++)
			{
				// Cave noise
				bool cave = false;
				for (int i = 0; i < caveSettingsLength; i++)
				{
					if (y + startY > caveSettings[i].maxHeight)
						continue;

					float noiseCaves = noise3D.eval(
						(float)((x + startX) * caveSettings[i].frequency) + caveSettings[i].offset,
						(float)((y + startY) * caveSettings[i].frequency) + caveSettings[i].offset,
						(float)((z + startZ) * caveSettings[i].frequency) + caveSettings[i].offset)
						* caveSettings[i].amplitude;

					if (noiseCaves > caveSettings[i].chance)
					{
						cave = true;
						break;
					}
				}

				// Step 1: Terrain Shape (surface and caves) and Ores

				// Sky and Caves
				if (y + startY > noiseY)
					chunkData->push_back(0);
				else if (cave)
					chunkData->push_back(0);
				// Ground
				else
				{
					bool blockSet = false;
					for (int i = 0; i < oreSettingsLength; i++)
					{
						if (y + startY > oreSettings[i].maxHeight)
							continue;

						float noiseOre = noise3D.eval(
							(float)((x + startX) * oreSettings[i].frequency) + oreSettings[i].offset,
							(float)((y + startY) * oreSettings[i].frequency) + oreSettings[i].offset,
							(float)((z + startZ) * oreSettings[i].frequency) + oreSettings[i].offset)
							* oreSettings[i].amplitude;

						if (noiseOre > oreSettings[i].chance)
						{
							chunkData->push_back(oreSettings[i].block);
							blockSet = true;
							break;
						}
					}

					if (!blockSet)
					{
						if (y + startY == noiseY)
							chunkData->push_back(Blocks::GRASS_BLOCK);
						else if (y + startY > 10)
							chunkData->push_back(Blocks::DIRT_BLOCK);
						else
							chunkData->push_back(Blocks::STONE_BLOCK);
					}
				}
			}
		}
	}
}