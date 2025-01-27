#include "Planet.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "WorldGen.h"

Planet* Planet::planet = nullptr;

//static const unsigned int CHUNK_SIZE = 32;

// Public
Planet::Planet(Shader* solidShader, Shader* waterShader, Shader* billboardShader)
	: solidShader(solidShader), waterShader(waterShader), billboardShader(billboardShader)
{
	chunkThread = std::thread(&Planet::ChunkThreadUpdate, this);
}

Planet::~Planet()
{
	shouldEnd = true;
	chunkThread.join();
}

void Planet::Update(glm::vec3 cameraPos)
{
	camChunkX = cameraPos.x < 0 ? floor(cameraPos.x / CHUNK_SIZE) : cameraPos.x / CHUNK_SIZE;
	camChunkY = cameraPos.y < 0 ? floor(cameraPos.y / CHUNK_SIZE) : cameraPos.y / CHUNK_SIZE;
	camChunkZ = cameraPos.z < 0 ? floor(cameraPos.z / CHUNK_SIZE) : cameraPos.z / CHUNK_SIZE;

	glDisable(GL_BLEND);

	chunksLoading = 0;
	numChunks = 0;
	numChunksRendered = 0;
	chunkMutex.lock();
	for (auto it = chunks.begin(); it != chunks.end(); )
	{
		numChunks++;

		Chunk& chunk = *it->second;

		if (!chunk.ready)
			chunksLoading++;

		int chunkX = chunk.chunkPos.x;
		int chunkY = chunk.chunkPos.y;
		int chunkZ = chunk.chunkPos.z;
		if ( chunk.ready && (abs(chunkX - camChunkX) > renderDistance ||
			abs(chunkY - camChunkY) > renderDistance ||
			abs(chunkZ - camChunkZ) > renderDistance))
		{
			// Out of range
			// Add chunk data to delete queue
			chunkDataDeleteQueue.push({ chunkX,     chunkY, chunkZ });
			chunkDataDeleteQueue.push({ chunkX + 1, chunkY, chunkZ });
			chunkDataDeleteQueue.push({ chunkX - 1, chunkY, chunkZ });
			chunkDataDeleteQueue.push({ chunkX, chunkY + 1, chunkZ });
			chunkDataDeleteQueue.push({ chunkX, chunkY - 1, chunkZ });
			chunkDataDeleteQueue.push({ chunkX, chunkY, chunkZ + 1 });
			chunkDataDeleteQueue.push({ chunkX, chunkY, chunkZ - 1 });

			// Delete chunk
			delete it->second; // &chunk
			it = chunks.erase(it);
		}
		else
		{
			numChunksRendered++;
			chunk.Render(solidShader, billboardShader);
			++it;
		}
	}

	glEnable(GL_BLEND);
	waterShader->use();
	for (auto it = chunks.begin(); it != chunks.end(); )
	{
		Chunk& chunk = *it->second;

		int chunkX = chunk.chunkPos.x;
		int chunkY = chunk.chunkPos.y;
		int chunkZ = chunk.chunkPos.z;

		chunk.RenderWater(waterShader);
		++it;
	}

	chunkMutex.unlock();
}

void Planet::ChunkThreadUpdate()
{
	while (!shouldEnd)
	{
		for (auto it = chunkData.begin(); it != chunkData.end(); )
		{
			ChunkPos pos = it->first;

			if (chunks.find(pos) == chunks.end() &&
				chunks.find({ pos.x + 1, pos.y, pos.z }) == chunks.end() &&
				chunks.find({ pos.x - 1, pos.y, pos.z }) == chunks.end() &&
				chunks.find({ pos.x, pos.y + 1, pos.z }) == chunks.end() &&
				chunks.find({ pos.x, pos.y - 1, pos.z }) == chunks.end() &&
				chunks.find({ pos.x, pos.y, pos.z + 1 }) == chunks.end() &&
				chunks.find({ pos.x, pos.y, pos.z - 1 }) == chunks.end())
			{
				chunkMutex.lock();
				delete it->second;
				it = chunkData.erase( it );
				chunkMutex.unlock();
			}
			else
			{
				++it;
			}
		}

		// Check if camera moved to new chunk
		if (camChunkX != lastCamX || camChunkY != lastCamY || camChunkZ != lastCamZ)
		{
			// Player moved chunks, start new chunk queue
			lastCamX = camChunkX;
			lastCamY = camChunkY;
			lastCamZ = camChunkZ;

			// Current chunk
			chunkMutex.lock();
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

			chunkMutex.unlock();
		}
		else
		{
			chunkMutex.lock();
			if (!chunkDataQueue.empty())
			{
				ChunkPos chunkPos = chunkDataQueue.front();

				if (chunkData.find(chunkPos) != chunkData.end())
				{
					chunkDataQueue.pop();
					chunkMutex.unlock();
					continue;
				}

				chunkMutex.unlock();

				uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
				ChunkData* data = new ChunkData(d);

				WorldGen::GenerateChunkData(chunkPos, d);

				chunkMutex.lock();
				chunkData[chunkPos] = data;
				chunkDataQueue.pop();
				chunkMutex.unlock();
			}
			else
			{
				if (!chunkQueue.empty())
				{
					// Check if chunk exists
					ChunkPos chunkPos = chunkQueue.front();
					if (chunks.find(chunkPos) != chunks.end())
					{
						chunkQueue.pop();
						chunkMutex.unlock();
						continue;
					}

					chunkMutex.unlock();

					// Create chunk object
					Chunk* chunk = new Chunk(chunkPos, solidShader, waterShader);

					// Set chunk data
					{
						chunkMutex.lock();
						if (chunkData.find(chunkPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(chunkPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->chunkData = data;

							chunkMutex.lock();
							chunkData[chunkPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->chunkData = chunkData.at(chunkPos);
							chunkMutex.unlock();
						}
					}

					// Set top data
					{
						ChunkPos topPos(chunkPos.x, chunkPos.y + 1, chunkPos.z);
						chunkMutex.lock();
						if (chunkData.find(topPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(topPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->upData = data;

							chunkMutex.lock();
							chunkData[topPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->upData = chunkData.at(topPos);
							chunkMutex.unlock();
						}
					}

					// Set bottom data
					{
						ChunkPos bottomPos(chunkPos.x, chunkPos.y - 1, chunkPos.z);
						chunkMutex.lock();
						if (chunkData.find(bottomPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(bottomPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->downData = data;

							chunkMutex.lock();
							chunkData[bottomPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->downData = chunkData.at(bottomPos);
							chunkMutex.unlock();
						}
					}

					// Set north data
					{
						ChunkPos northPos(chunkPos.x, chunkPos.y, chunkPos.z - 1);
						chunkMutex.lock();
						if (chunkData.find(northPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(northPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->northData = data;

							chunkMutex.lock();
							chunkData[northPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->northData = chunkData.at(northPos);
							chunkMutex.unlock();
						}
					}

					// Set south data
					{
						ChunkPos southPos(chunkPos.x, chunkPos.y, chunkPos.z + 1);
						chunkMutex.lock();
						if (chunkData.find(southPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(southPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->southData = data;

							chunkMutex.lock();
							chunkData[southPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->southData = chunkData.at(southPos);
							chunkMutex.unlock();
						}
					}

					// Set east data
					{
						ChunkPos eastPos(chunkPos.x + 1, chunkPos.y, chunkPos.z);
						chunkMutex.lock();
						if (chunkData.find(eastPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(eastPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->eastData = data;

							chunkMutex.lock();
							chunkData[eastPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->eastData = chunkData.at(eastPos);
							chunkMutex.unlock();
						}
					}

					// Set west data
					{
						ChunkPos westPos(chunkPos.x - 1, chunkPos.y, chunkPos.z);
						chunkMutex.lock();
						if (chunkData.find(westPos) == chunkData.end())
						{
							chunkMutex.unlock();
							uint16_t* d = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

							WorldGen::GenerateChunkData(westPos, d);

							ChunkData* data = new ChunkData(d);

							chunk->westData = data;

							chunkMutex.lock();
							chunkData[westPos] = data;
							chunkMutex.unlock();
						}
						else
						{
							chunk->westData = chunkData.at(westPos);
							chunkMutex.unlock();
						}
					}

					// Generate chunk mesh
					chunk->GenerateChunkMesh();

					// Finish
					chunkMutex.lock();
					chunks[chunkPos] = chunk;
					chunkQueue.pop();
					chunkMutex.unlock();
				}
				else
				{
					chunkMutex.unlock();

					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}
		}
	}
}

Chunk* Planet::GetChunk(ChunkPos chunkPos)
{
	chunkMutex.lock();
	if (chunks.find(chunkPos) == chunks.end())
	{
		chunkMutex.unlock();
		return nullptr;
	}
	else
	{
		chunkMutex.unlock();
		return chunks.at(chunkPos);
	}
}

void Planet::ClearChunkQueue()
{
	lastCamX++;
}