#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <glm/glm.hpp>

#include "Chunk.h"
#include "TupleHash.h"

class Planet
{
// Methods
public:
	Planet();
	~Planet();

	std::vector<unsigned int> GetChunkData(int chunkX, int chunkY, int chunkZ);
	void Update(float camX, float camY, float camZ, unsigned int modelLoc);

// Variables
public:
	static Planet* planet;
	unsigned int numChunks = 0, numChunksRendered = 0;

private:
	std::unordered_map<std::tuple<int, int, int>, Chunk> chunks;
	std::queue<glm::vec3> chunkQueue;
	int renderDistance = 3;
	int renderHeight = 1;
	unsigned int chunkSize = 32;
	unsigned int chunksLoading = 0;
	int lastCamX = -100, lastCamY = -100, lastCamZ = -100;
};