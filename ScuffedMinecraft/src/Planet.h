#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>

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

	ChunkData* GetChunkData(glm::ivec3& chunkPos);
	void Update(glm::vec3 cameraPos);

	Chunk* GetChunk(const glm::ivec3& chunkPos);
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
	std::unordered_map<glm::ivec3, Chunk*, ChunkPosHash> chunks;
	std::unordered_map<glm::ivec3, ChunkData*, ChunkPosHash> chunkData;
	std::queue<glm::ivec3> chunkQueue;
	std::queue<glm::ivec3> chunkDataQueue;
	std::queue<glm::ivec3> chunkDataDeleteQueue;
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