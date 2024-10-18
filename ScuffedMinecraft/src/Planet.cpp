#include "Planet.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Planet* Planet::planet = nullptr;

// Public
Planet::Planet(Shader* solidShader, Shader* waterShader, Shader* billboardShader)
	: solidShader(solidShader), waterShader(waterShader), billboardShader(billboardShader)
{

}

Planet::~Planet()
{

}

std::vector<unsigned int> Planet::GetChunkData(int chunkX, int chunkY, int chunkZ)
{
	std::tuple<int, int, int> chunkTuple{ chunkX, chunkY, chunkZ };

	if (chunks.find(chunkTuple) == chunks.end())
	{
		return std::vector<unsigned int>{};
		//return GenerateChunkData(chunkX, chunkY, chunkZ);
	}
	else
	{
		return std::vector<unsigned int>{};
		//return chunks.at(chunkTuple).chunkData;
	}
}

void Planet::Update(float camX, float camY, float camZ)
{
	int camChunkX = camX < 0 ? floor(camX / chunkSize) : camX / chunkSize;
	int camChunkY = camY < 0 ? floor(camY / chunkSize) : camY / chunkSize;
	int camChunkZ = camZ < 0 ? floor(camZ / chunkSize) : camZ / chunkSize;

	// Check if camera moved to new chunk
	if (camChunkX != lastCamX || camChunkY != lastCamY || camChunkZ != lastCamZ)
	{
		// Player moved chunks, start new chunk queue
		lastCamX = camChunkX;
		lastCamY = camChunkY;
		lastCamZ = camChunkZ;

		// Current chunk
		chunkQueue = {};
		if (chunks.find({ camChunkX, camChunkY, camChunkZ }) == chunks.end())
			chunkQueue.push({ camChunkX, camChunkY, camChunkZ });

		for (int r = 0; r < renderDistance; r++)
		{
			// Add middle chunks
			for (int y = 0; y <= renderHeight; y++)
			{
				chunkQueue.push({ camChunkX,     camChunkY + y, camChunkZ + r });
				chunkQueue.push({ camChunkX + r, camChunkY + y, camChunkZ });
				chunkQueue.push({ camChunkX,     camChunkY + y, camChunkZ - r });
				chunkQueue.push({ camChunkX - r, camChunkY + y, camChunkZ });

				if (y > 0)
				{
					chunkQueue.push({ camChunkX,     camChunkY - y, camChunkZ + r });
					chunkQueue.push({ camChunkX + r, camChunkY - y, camChunkZ });
					chunkQueue.push({ camChunkX,     camChunkY - y, camChunkZ - r });
					chunkQueue.push({ camChunkX - r, camChunkY - y, camChunkZ });
				}
			}

			// Add edges
			for (int e = 1; e < r; e++)
			{
				for (int y = 0; y <= renderHeight; y++)
				{
					chunkQueue.push({ camChunkX + e, camChunkY + y, camChunkZ + r });
					chunkQueue.push({ camChunkX - e, camChunkY + y, camChunkZ + r });

					chunkQueue.push({ camChunkX + r, camChunkY + y, camChunkZ + e });
					chunkQueue.push({ camChunkX + r, camChunkY + y, camChunkZ - e });

					chunkQueue.push({ camChunkX + e, camChunkY + y, camChunkZ - r });
					chunkQueue.push({ camChunkX - e, camChunkY + y, camChunkZ - r });

					chunkQueue.push({ camChunkX - r, camChunkY + y, camChunkZ + e });
					chunkQueue.push({ camChunkX - r, camChunkY + y, camChunkZ - e });

					if (y > 0)
					{
						chunkQueue.push({ camChunkX + e, camChunkY - y, camChunkZ + r });
						chunkQueue.push({ camChunkX - e, camChunkY - y, camChunkZ + r });

						chunkQueue.push({ camChunkX + r, camChunkY - y, camChunkZ + e });
						chunkQueue.push({ camChunkX + r, camChunkY - y, camChunkZ - e });

						chunkQueue.push({ camChunkX + e, camChunkY - y, camChunkZ - r });
						chunkQueue.push({ camChunkX - e, camChunkY - y, camChunkZ - r });

						chunkQueue.push({ camChunkX - r, camChunkY - y, camChunkZ + e });
						chunkQueue.push({ camChunkX - r, camChunkY - y, camChunkZ - e });
					}
				}
			}

			// Add corners
			for (int y = 0; y <= renderHeight; y++)
			{
				chunkQueue.push({ camChunkX + r, camChunkY + y, camChunkZ + r });
				chunkQueue.push({ camChunkX + r, camChunkY + y, camChunkZ - r });
				chunkQueue.push({ camChunkX - r, camChunkY + y, camChunkZ + r });
				chunkQueue.push({ camChunkX - r, camChunkY + y, camChunkZ - r });

				if (y > 0)
				{
					chunkQueue.push({ camChunkX + r, camChunkY - y, camChunkZ + r });
					chunkQueue.push({ camChunkX + r, camChunkY - y, camChunkZ - r });
					chunkQueue.push({ camChunkX - r, camChunkY - y, camChunkZ + r });
					chunkQueue.push({ camChunkX - r, camChunkY - y, camChunkZ - r });
				}
			}
		}
	}
	else if (chunksLoading == 0 && !chunkQueue.empty())
	{
		// Queue is not empty. Process front item in queue
		glm::vec3 next = chunkQueue.front();
		chunkQueue.pop();

		std::tuple<int, int, int> chunkTuple{ next.x, next.y, next.z };

		if (chunks.find(chunkTuple) == chunks.end())
		{
			chunks.try_emplace(chunkTuple,
				chunkSize, next, solidShader, waterShader
			);
		}
	}

	glDisable(GL_BLEND);

	chunksLoading = 0;
	numChunks = 0;
	numChunksRendered = 0;
	for (auto it = chunks.begin(); it != chunks.end(); )
	{
		numChunks++;

		if (!it->second.ready)
			chunksLoading++;

		int chunkX = it->second.chunkPos.x;
		int chunkY = it->second.chunkPos.y;
		int chunkZ = it->second.chunkPos.z;
		if (it->second.ready && (abs(chunkX - camChunkX) > renderDistance ||
			abs(chunkY - camChunkY) > renderDistance ||
			abs(chunkZ - camChunkZ) > renderDistance))
		{
			it = chunks.erase(it);
		}
		else
		{
			numChunksRendered++;
			it->second.Render(solidShader, billboardShader);
			++it;
		}
	}

	glEnable(GL_BLEND);
	waterShader->use();
	for (auto it = chunks.begin(); it != chunks.end(); )
	{
		int chunkX = it->second.chunkPos.x;
		int chunkY = it->second.chunkPos.y;
		int chunkZ = it->second.chunkPos.z;

		it->second.RenderWater(waterShader);
		++it;
	}
}

Chunk* Planet::GetChunk(int chunkX, int chunkY, int chunkZ)
{
	std::tuple<int, int, int> chunkTuple{ chunkX, chunkY, chunkZ };

	if (chunks.find(chunkTuple) == chunks.end())
	{
		return nullptr;
	}
	else
	{
		return &chunks.at(chunkTuple);
	}
}