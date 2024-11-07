#pragma once

#include <vector>
#include <thread>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Vertex.h"
#include "ChunkData.h"

class Chunk
{
public:
	Chunk(const glm::ivec3& chunkPos, Shader* shader, Shader* waterShader);
	~Chunk();

	void GenerateChunkMesh();
	void Render(Shader* mainShader, Shader* billboardShader);
	void RenderWater(Shader* shader);
	uint16_t GetBlockAtPos(int x, int y, int z);
	void UpdateBlock(int x, int y, int z, uint16_t newBlock);
	void UpdateChunk();

public:
	ChunkData* chunkData;
	ChunkData* northData;
	ChunkData* southData;
	ChunkData* upData;
	ChunkData* downData;
	ChunkData* eastData;
	ChunkData* westData;
	glm::ivec3 chunkPos; 
	bool ready;
	bool generated;

private:
	glm::vec3 worldPos;
	std::thread chunkThread;

	std::vector<Vertex> mainVertices;
	std::vector<unsigned int> mainIndices;
	std::vector<WaterVertex> waterVertices;
	std::vector<unsigned int> waterIndices;
	std::vector<BillboardVertex> billboardVertices;
	std::vector<unsigned int> billboardIndices;

	unsigned int mainVAO, waterVAO, billboardVAO;
	unsigned int mainVBO, mainEBO, waterVBO, waterEBO, billboardVBO, billboardEBO;
	unsigned int numTrianglesMain, numTrianglesWater, numTrianglesBillboard;
	unsigned int modelLoc;
};