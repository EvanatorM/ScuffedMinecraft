#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>

#include "ChunkPos.h"
#include "ChunkData.h"
#include "Chunk.h"
#include "ChunkPosHash.h"

constexpr unsigned int CHUNK_SIZE = 32;

class Planet
{
// Methods
public:
	Planet(Shader* solidShader, Shader* waterShader, Shader* billboardShader);
	~Planet();

	ChunkData* GetChunkData(ChunkPos chunkPos);
	void Update(glm::vec3 cameraPos);

	Chunk* GetChunk(ChunkPos chunkPos);
	void ClearChunkQueue();

private:
	void ChunkThreadUpdate();

// Variables
public:
	static Planet* planet;
	unsigned int numChunks = 0, numChunksRendered = 0;
	int renderDistance = 10;
	int renderHeight = 3;

private:
	std::unordered_map<ChunkPos, Chunk*, ChunkPosHash> chunks;
	std::unordered_map<ChunkPos, ChunkData*, ChunkPosHash> chunkData;
	std::queue<ChunkPos> chunkQueue;
	std::queue<ChunkPos> chunkDataQueue;
	std::queue<ChunkPos> chunkDataDeleteQueue;
	unsigned int chunksLoading = 0;
	int lastCamX = -100, lastCamY = -100, lastCamZ = -100;
	int camChunkX = -100, camChunkY = -100, camChunkZ = -100;

	Shader* solidShader;
	Shader* waterShader;
	Shader* billboardShader;

	std::thread chunkThread;
	std::mutex chunkMutex;

	bool shouldEnd = false;
};