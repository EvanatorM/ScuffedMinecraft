#pragma once

#include <vector>
#include <thread>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Vertex.h"

class Chunk
{
public:
	Chunk(unsigned int chunkSize, glm::vec3 chunkPos, Shader* shader, Shader* waterShader);
	~Chunk();

	void GenerateChunk();
	void Render(Shader* mainShader, Shader* billboardShader);
	void RenderWater(Shader* shader);
	unsigned int GetBlockAtPos(int x, int y, int z);

public:
	std::vector<unsigned int> chunkData;
	glm::vec3 chunkPos;
	bool ready;
	bool generated;

private:
	unsigned int chunkSize;
	glm::vec3 worldPos;
	std::thread chunkThread;

	std::vector<Vertex> mainVertices;
	std::vector<unsigned int> mianIndices;
	std::vector<WaterVertex> waterVertices;
	std::vector<unsigned int> waterIndices;
	std::vector<BillboardVertex> billboardVertices;
	std::vector<unsigned int> billboardIndices;

	unsigned int mainVAO, waterVAO, billboardVAO;
	unsigned int mainVBO, mainEBO, waterVBO, waterEBO, billboardVBO, billboardEBO;
	unsigned int numTrianglesMain, numTrianglesWater, numTrianglesBillboard;
	unsigned int modelLoc;
};