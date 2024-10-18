#include "Chunk.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Planet.h"
#include "Blocks.h"
#include "WorldGen.h"

Chunk::Chunk(unsigned int chunkSize, glm::vec3 chunkPos, Shader* shader, Shader* waterShader)
	: chunkSize(chunkSize), chunkPos(chunkPos)
{
	worldPos = glm::vec3(chunkPos.x * chunkSize, chunkPos.y * chunkSize, chunkPos.z * chunkSize);

	ready = false;
	generated = false;
	chunkThread = std::thread(&Chunk::GenerateChunk, this);
}

Chunk::~Chunk()
{
	if (chunkThread.joinable())
		chunkThread.join();

	glDeleteBuffers(1, &mainVBO);
	glDeleteBuffers(1, &mainEBO);
	glDeleteVertexArrays(1, &mainVAO);

	glDeleteBuffers(1, &waterVBO);
	glDeleteBuffers(1, &waterEBO);
	glDeleteVertexArrays(1, &waterVAO);

	glDeleteBuffers(1, &billboardVBO);
	glDeleteBuffers(1, &billboardEBO);
	glDeleteVertexArrays(1, &billboardVAO);
}

void Chunk::GenerateChunk()
{
	//std::cout << "Started thread: " << std::this_thread::get_id() << '\n';

	WorldGen::GenerateChunkData(chunkPos.x, chunkPos.y, chunkPos.z, chunkSize, &chunkData);
	std::vector<unsigned int> northData, southData, eastData, westData, upData, downData;

	WorldGen::GenerateChunkData(chunkPos.x,     chunkPos.y,     chunkPos.z - 1, chunkSize, &northData);
	WorldGen::GenerateChunkData(chunkPos.x,     chunkPos.y,     chunkPos.z + 1, chunkSize, &southData);
	WorldGen::GenerateChunkData(chunkPos.x + 1, chunkPos.y,     chunkPos.z, chunkSize, &eastData);
	WorldGen::GenerateChunkData(chunkPos.x - 1, chunkPos.y,     chunkPos.z, chunkSize, &westData);
	WorldGen::GenerateChunkData(chunkPos.x,     chunkPos.y + 1, chunkPos.z, chunkSize, &upData);
	WorldGen::GenerateChunkData(chunkPos.x,     chunkPos.y - 1, chunkPos.z, chunkSize, &downData);

	//std::cout << "Got chunk data in thread: " << std::this_thread::get_id() << '\n';

	unsigned int currentVertex = 0;
	unsigned int currentLiquidVertex = 0;
	unsigned int currentBillboardVertex = 0;
	for (char x = 0; x < chunkSize; x++)
	{
		for (char z = 0; z < chunkSize; z++)
		{
			for (char y = 0; y < chunkSize; y++)
			{
				int index = x * chunkSize * chunkSize + z * chunkSize + y;
				if (chunkData[index] == 0)
					continue;

				const Block* block = &Blocks::blocks[chunkData[index]];

				int topBlock;
				if (y < chunkSize - 1)
				{
					int blockIndex = x * chunkSize * chunkSize + z * chunkSize + (y + 1);
					topBlock = chunkData[blockIndex];
				}
				else
				{
					int blockIndex = x * chunkSize * chunkSize + z * chunkSize + 0;
					topBlock = upData[blockIndex];
				}

				const Block* topBlockType = &Blocks::blocks[topBlock];
				char waterTopValue = topBlockType->blockType == Block::TRANSPARENT ? 1 : 0;

				if (block->blockType == Block::BILLBOARD)
				{
					billboardVertices.push_back(BillboardVertex(x + .85355f, y + 0, z + .85355f, block->sideMinX, block->sideMinY));
					billboardVertices.push_back(BillboardVertex(x + .14645f, y + 0, z + .14645f, block->sideMaxX, block->sideMinY));
					billboardVertices.push_back(BillboardVertex(x + .85355f, y + 1, z + .85355f, block->sideMinX, block->sideMaxY));
					billboardVertices.push_back(BillboardVertex(x + .14645f, y + 1, z + .14645f, block->sideMaxX, block->sideMaxY));

					billboardIndices.push_back(currentBillboardVertex + 0);
					billboardIndices.push_back(currentBillboardVertex + 3);
					billboardIndices.push_back(currentBillboardVertex + 1);
					billboardIndices.push_back(currentBillboardVertex + 0);
					billboardIndices.push_back(currentBillboardVertex + 2);
					billboardIndices.push_back(currentBillboardVertex + 3);
					currentBillboardVertex += 4;

					billboardVertices.push_back(BillboardVertex(x + .14645f, y + 0, z + .85355f, block->sideMinX, block->sideMinY));
					billboardVertices.push_back(BillboardVertex(x + .85355f, y + 0, z + .14645f, block->sideMaxX, block->sideMinY));
					billboardVertices.push_back(BillboardVertex(x + .14645f, y + 1, z + .85355f, block->sideMinX, block->sideMaxY));
					billboardVertices.push_back(BillboardVertex(x + .85355f, y + 1, z + .14645f, block->sideMaxX, block->sideMaxY));

					billboardIndices.push_back(currentBillboardVertex + 0);
					billboardIndices.push_back(currentBillboardVertex + 3);
					billboardIndices.push_back(currentBillboardVertex + 1);
					billboardIndices.push_back(currentBillboardVertex + 0);
					billboardIndices.push_back(currentBillboardVertex + 2);
					billboardIndices.push_back(currentBillboardVertex + 3);
					currentBillboardVertex += 4;
				}
				else
				{
					// North
					{
						int northBlock;
						if (z > 0)
						{
							int northIndex = x * chunkSize * chunkSize + (z - 1) * chunkSize + y;
							northBlock = chunkData[northIndex];
						}
						else
						{
							int northIndex = x * chunkSize * chunkSize + (chunkSize - 1) * chunkSize + y;
							northBlock = northData[northIndex];
						}

						const Block* northBlockType = &Blocks::blocks[northBlock];

						if (northBlockType->blockType == Block::LEAVES
							|| northBlockType->blockType == Block::TRANSPARENT
							|| northBlockType->blockType == Block::BILLBOARD
							|| (northBlockType->blockType == Block::LIQUID && block->blockType != Block::LIQUID))
						{
							if (block->blockType == Block::LIQUID)
							{
								waterVertices.push_back(WaterVertex(x + 1, y + 0, z + 0, block->sideMinX, block->sideMinY, 0, 0));
								waterVertices.push_back(WaterVertex(x + 0, y + 0, z + 0, block->sideMaxX, block->sideMinY, 0, 0));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 0, block->sideMinX, block->sideMaxY, 0, waterTopValue));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 0, waterTopValue));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.push_back(Vertex(x + 1, y + 0, z + 0, block->sideMinX, block->sideMinY, 0));
								mainVertices.push_back(Vertex(x + 0, y + 0, z + 0, block->sideMaxX, block->sideMinY, 0));
								mainVertices.push_back(Vertex(x + 1, y + 1, z + 0, block->sideMinX, block->sideMaxY, 0));
								mainVertices.push_back(Vertex(x + 0, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 0));

								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 3);
								mianIndices.push_back(currentVertex + 1);
								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 2);
								mianIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// South
					{
						int southBlock;
						if (z < chunkSize - 1)
						{
							int southIndex = x * chunkSize * chunkSize + (z + 1) * chunkSize + y;
							southBlock = chunkData[southIndex];
						}
						else
						{
							int southIndex = x * chunkSize * chunkSize + 0 * chunkSize + y;
							southBlock = southData[southIndex];
						}

						const Block* southBlockType = &Blocks::blocks[southBlock];

						if (southBlockType->blockType == Block::LEAVES 
							|| southBlockType->blockType == Block::TRANSPARENT
							|| southBlockType->blockType == Block::BILLBOARD
							|| (southBlockType->blockType == Block::LIQUID && block->blockType != Block::LIQUID))
						{
							if (block->blockType == Block::LIQUID)
							{
								waterVertices.push_back(WaterVertex(x + 0, y + 0, z + 1, block->sideMinX, block->sideMinY, 1, 0));
								waterVertices.push_back(WaterVertex(x + 1, y + 0, z + 1, block->sideMaxX, block->sideMinY, 1, 0));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 1, block->sideMinX, block->sideMaxY, 1, waterTopValue));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 1, waterTopValue));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.push_back(Vertex(x + 0, y + 0, z + 1, block->sideMinX, block->sideMinY, 1));
								mainVertices.push_back(Vertex(x + 1, y + 0, z + 1, block->sideMaxX, block->sideMinY, 1));
								mainVertices.push_back(Vertex(x + 0, y + 1, z + 1, block->sideMinX, block->sideMaxY, 1));
								mainVertices.push_back(Vertex(x + 1, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 1));

								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 3);
								mianIndices.push_back(currentVertex + 1);
								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 2);
								mianIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// West
					{
						int westBlock;
						if (x > 0)
						{
							int blockIndex = (x - 1) * chunkSize * chunkSize + z * chunkSize + y;
							westBlock = chunkData[blockIndex];
						}
						else
						{
							int blockIndex = (chunkSize - 1) * chunkSize * chunkSize + z * chunkSize + y;
							westBlock = westData[blockIndex];
						}

						const Block* westBlockType = &Blocks::blocks[westBlock];

						if (westBlockType->blockType == Block::LEAVES 
							|| westBlockType->blockType == Block::TRANSPARENT
							|| westBlockType->blockType == Block::BILLBOARD
							|| (westBlockType->blockType == Block::LIQUID && block->blockType != Block::LIQUID))
						{
							if (block->blockType == Block::LIQUID)
							{
								waterVertices.push_back(WaterVertex(x + 0, y + 0, z + 0, block->sideMinX, block->sideMinY, 2, 0));
								waterVertices.push_back(WaterVertex(x + 0, y + 0, z + 1, block->sideMaxX, block->sideMinY, 2, 0));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 0, block->sideMinX, block->sideMaxY, 2, waterTopValue));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 2, waterTopValue));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.push_back(Vertex(x + 0, y + 0, z + 0, block->sideMinX, block->sideMinY, 2));
								mainVertices.push_back(Vertex(x + 0, y + 0, z + 1, block->sideMaxX, block->sideMinY, 2));
								mainVertices.push_back(Vertex(x + 0, y + 1, z + 0, block->sideMinX, block->sideMaxY, 2));
								mainVertices.push_back(Vertex(x + 0, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 2));

								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 3);
								mianIndices.push_back(currentVertex + 1);
								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 2);
								mianIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// East
					{
						int eastBlock;
						if (x < chunkSize - 1)
						{
							int blockIndex = (x + 1) * chunkSize * chunkSize + z * chunkSize + y;
							eastBlock = chunkData[blockIndex];
						}
						else
						{
							int blockIndex = 0 * chunkSize * chunkSize + z * chunkSize + y;
							eastBlock = eastData[blockIndex];
						}

						const Block* eastBlockType = &Blocks::blocks[eastBlock];

						if (eastBlockType->blockType == Block::LEAVES 
							|| eastBlockType->blockType == Block::TRANSPARENT
							|| eastBlockType->blockType == Block::BILLBOARD
							|| (eastBlockType->blockType == Block::LIQUID && block->blockType != Block::LIQUID))
						{
							if (block->blockType == Block::LIQUID)
							{
								waterVertices.push_back(WaterVertex(x + 1, y + 0, z + 1, block->sideMinX, block->sideMinY, 3, 0));
								waterVertices.push_back(WaterVertex(x + 1, y + 0, z + 0, block->sideMaxX, block->sideMinY, 3, 0));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 1, block->sideMinX, block->sideMaxY, 3, waterTopValue));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 3, waterTopValue));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.push_back(Vertex(x + 1, y + 0, z + 1, block->sideMinX, block->sideMinY, 3));
								mainVertices.push_back(Vertex(x + 1, y + 0, z + 0, block->sideMaxX, block->sideMinY, 3));
								mainVertices.push_back(Vertex(x + 1, y + 1, z + 1, block->sideMinX, block->sideMaxY, 3));
								mainVertices.push_back(Vertex(x + 1, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 3));

								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 3);
								mianIndices.push_back(currentVertex + 1);
								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 2);
								mianIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// Bottom
					{
						int bottomBlock;
						if (y > 0)
						{
							int blockIndex = x * chunkSize * chunkSize + z * chunkSize + (y - 1);
							bottomBlock = chunkData[blockIndex];
						}
						else
						{
							int blockIndex = x * chunkSize * chunkSize + z * chunkSize + (chunkSize - 1);
							bottomBlock = downData[blockIndex];
						}

						const Block* bottomBlockType = &Blocks::blocks[bottomBlock];

						if (bottomBlockType->blockType == Block::LEAVES 
							|| bottomBlockType->blockType == Block::TRANSPARENT
							|| bottomBlockType->blockType == Block::BILLBOARD
							|| (bottomBlockType->blockType == Block::LIQUID && block->blockType != Block::LIQUID))
						{
							if (block->blockType == Block::LIQUID)
							{
								waterVertices.push_back(WaterVertex(x + 1, y + 0, z + 1, block->bottomMinX, block->bottomMinY, 4, 0));
								waterVertices.push_back(WaterVertex(x + 0, y + 0, z + 1, block->bottomMaxX, block->bottomMinY, 4, 0));
								waterVertices.push_back(WaterVertex(x + 1, y + 0, z + 0, block->bottomMinX, block->bottomMaxY, 4, 0));
								waterVertices.push_back(WaterVertex(x + 0, y + 0, z + 0, block->bottomMaxX, block->bottomMaxY, 4, 0));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.push_back(Vertex(x + 1, y + 0, z + 1, block->bottomMinX, block->bottomMinY, 4));
								mainVertices.push_back(Vertex(x + 0, y + 0, z + 1, block->bottomMaxX, block->bottomMinY, 4));
								mainVertices.push_back(Vertex(x + 1, y + 0, z + 0, block->bottomMinX, block->bottomMaxY, 4));
								mainVertices.push_back(Vertex(x + 0, y + 0, z + 0, block->bottomMaxX, block->bottomMaxY, 4));

								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 3);
								mianIndices.push_back(currentVertex + 1);
								mianIndices.push_back(currentVertex + 0);
								mianIndices.push_back(currentVertex + 2);
								mianIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// Top
					{
						if (block->blockType == Block::LIQUID)
						{
							if (topBlockType->blockType != Block::LIQUID)
							{
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 1, block->topMinX, block->topMinY, 5, 1));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 1, block->topMaxX, block->topMinY, 5, 1));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 0, block->topMinX, block->topMaxY, 5, 1));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 0, block->topMaxX, block->topMaxY, 5, 1));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;

								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 1, block->topMinX, block->topMinY, 5, 1));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 1, block->topMaxX, block->topMinY, 5, 1));
								waterVertices.push_back(WaterVertex(x + 1, y + 1, z + 0, block->topMinX, block->topMaxY, 5, 1));
								waterVertices.push_back(WaterVertex(x + 0, y + 1, z + 0, block->topMaxX, block->topMaxY, 5, 1));

								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 3);
								waterIndices.push_back(currentLiquidVertex + 1);
								waterIndices.push_back(currentLiquidVertex + 0);
								waterIndices.push_back(currentLiquidVertex + 2);
								waterIndices.push_back(currentLiquidVertex + 3);
								currentLiquidVertex += 4;
							}
						}
						else if (topBlockType->blockType == Block::LEAVES
							|| topBlockType->blockType == Block::TRANSPARENT
							|| topBlockType->blockType == Block::BILLBOARD
							|| topBlockType->blockType == Block::LIQUID)
						{
							mainVertices.push_back(Vertex(x + 0, y + 1, z + 1, block->topMinX, block->topMinY, 5));
							mainVertices.push_back(Vertex(x + 1, y + 1, z + 1, block->topMaxX, block->topMinY, 5));
							mainVertices.push_back(Vertex(x + 0, y + 1, z + 0, block->topMinX, block->topMaxY, 5));
							mainVertices.push_back(Vertex(x + 1, y + 1, z + 0, block->topMaxX, block->topMaxY, 5));

							mianIndices.push_back(currentVertex + 0);
							mianIndices.push_back(currentVertex + 3);
							mianIndices.push_back(currentVertex + 1);
							mianIndices.push_back(currentVertex + 0);
							mianIndices.push_back(currentVertex + 2);
							mianIndices.push_back(currentVertex + 3);
							currentVertex += 4;
						}
					}
				}
			}
		}
	}

	//std::cout << "Finished generating in thread: " << std::this_thread::get_id() << '\n';

	generated = true;

	//std::cout << "Generated: " << generated << '\n';
}

void Chunk::Render(Shader* mainShader, Shader* billboardShader)
{
	if (!ready)
	{
		if (generated)
		{
			// Solid
			numTrianglesMain = mianIndices.size();

			glGenVertexArrays(1, &mainVAO);
			glGenBuffers(1, &mainVBO);
			glGenBuffers(1, &mainEBO);

			glBindVertexArray(mainVAO);

			glBindBuffer(GL_ARRAY_BUFFER, mainVBO);
			glBufferData(GL_ARRAY_BUFFER, mainVertices.size() * sizeof(Vertex), mainVertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mianIndices.size() * sizeof(unsigned int), mianIndices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, posX));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texGridX));
			glEnableVertexAttribArray(1);
			glVertexAttribIPointer(2, 1, GL_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, direction));
			glEnableVertexAttribArray(2); 
			
			// Water
			numTrianglesWater = waterIndices.size();

			glGenVertexArrays(1, &waterVAO);
			glGenBuffers(1, &waterVBO);
			glGenBuffers(1, &waterEBO);

			glBindVertexArray(waterVAO);

			glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
			glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(WaterVertex), waterVertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int), waterIndices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, sizeof(WaterVertex), (void*)offsetof(WaterVertex, posX));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_BYTE, GL_FALSE, sizeof(WaterVertex), (void*)offsetof(WaterVertex, texGridX));
			glEnableVertexAttribArray(1);
			glVertexAttribIPointer(2, 1, GL_BYTE, sizeof(WaterVertex), (void*)offsetof(WaterVertex, direction));
			glEnableVertexAttribArray(2);
			glVertexAttribIPointer(3, 1, GL_BYTE, sizeof(WaterVertex), (void*)offsetof(WaterVertex, top));
			glEnableVertexAttribArray(3);
			ready = true;

			// Billboard
			numTrianglesBillboard = billboardIndices.size();

			glGenVertexArrays(1, &billboardVAO);
			glGenBuffers(1, &billboardVBO);
			glGenBuffers(1, &billboardEBO);

			glBindVertexArray(billboardVAO);

			glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
			glBufferData(GL_ARRAY_BUFFER, billboardVertices.size() * sizeof(BillboardVertex), billboardVertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, billboardIndices.size() * sizeof(unsigned int), billboardIndices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BillboardVertex), (void*)offsetof(BillboardVertex, posX));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_BYTE, GL_FALSE, sizeof(BillboardVertex), (void*)offsetof(BillboardVertex, texGridX));
			glEnableVertexAttribArray(1);
			ready = true;
		}
		
		return;
	}

	//std::cout << "Rendering chunk " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << '\n'
	//	<< "Chunk VAO: " << vertexArrayObject << '\n' << "Triangles: " << numTriangles << '\n';

	// Calculate model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, worldPos);

	// Render main mesh
	mainShader->use();

	modelLoc = glGetUniformLocation(mainShader->ID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(mainVAO);
	glDrawElements(GL_TRIANGLES, numTrianglesMain, GL_UNSIGNED_INT, 0);

	// Render billboard mesh
	billboardShader->use(); 

	modelLoc = glGetUniformLocation(billboardShader->ID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDisable(GL_CULL_FACE);
	glBindVertexArray(billboardVAO);
	glDrawElements(GL_TRIANGLES, numTrianglesBillboard, GL_UNSIGNED_INT, 0);
	glEnable(GL_CULL_FACE);
}

void Chunk::RenderWater(Shader* shader)
{
	if (!ready)
		return;

	//std::cout << "Rendering chunk " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << '\n'
	//	<< "Chunk VAO: " << vertexArrayObject << '\n' << "Triangles: " << numTriangles << '\n';

	modelLoc = glGetUniformLocation(shader->ID, "model");
	glBindVertexArray(waterVAO);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, worldPos);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, numTrianglesWater, GL_UNSIGNED_INT, 0);
}

unsigned int Chunk::GetBlockAtPos(int x, int y, int z)
{
	if (!ready)
		return 0;

	int index = x * chunkSize * chunkSize + z * chunkSize + y;
	return chunkData[index];
}