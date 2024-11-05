#include "Chunk.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Planet.h"
#include "Blocks.h"
#include "WorldGen.h"

Chunk::Chunk(ChunkPos chunkPos, Shader* shader, Shader* waterShader)
	: chunkPos(chunkPos)
{
	worldPos = glm::vec3(chunkPos.x * (float)CHUNK_SIZE, chunkPos.y * (float)CHUNK_SIZE, chunkPos.z * (float)CHUNK_SIZE);

	ready = false;
	generated = false;
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

void Chunk::GenerateChunkMesh()
{
	mainVertices.clear();
	mainIndices.clear();
	waterVertices.clear();
	waterIndices.clear();
	billboardVertices.clear();
	billboardIndices.clear();
	numTrianglesMain = 0;
	numTrianglesWater = 0;
	numTrianglesBillboard = 0;

	unsigned int currentVertex = 0;
	unsigned int currentLiquidVertex = 0;
	unsigned int currentBillboardVertex = 0;
	for (char x = 0; x < CHUNK_SIZE; x++)
	{
		for (char z = 0; z < CHUNK_SIZE; z++)
		{
			for (char y = 0; y < CHUNK_SIZE; y++)
			{
				if (chunkData->GetBlock(x, y, z) == 0)
					continue;

				const Block* block = &Blocks::blocks[chunkData->GetBlock(x, y, z)];

				int topBlock;
				if (y < CHUNK_SIZE - 1)
				{
					topBlock = chunkData->GetBlock(x, y + 1, z);
				}
				else
				{
					int blockIndex = x * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + 0;
					topBlock = upData->GetBlock(x, 0, z);
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
							northBlock = chunkData->GetBlock(x, y, z - 1);
						}
						else
						{
							northBlock = northData->GetBlock(x, y, CHUNK_SIZE - 1);
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

								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 3);
								mainIndices.push_back(currentVertex + 1);
								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 2);
								mainIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// South
					{
						int southBlock;
						if (z < CHUNK_SIZE - 1)
						{
							southBlock = chunkData->GetBlock(x, y, z + 1);
						}
						else
						{
							southBlock = southData->GetBlock(x, y, 0);
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

								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 3);
								mainIndices.push_back(currentVertex + 1);
								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 2);
								mainIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// West
					{
						int westBlock;
						if (x > 0)
						{
							westBlock = chunkData->GetBlock(x - 1, y, z);
						}
						else
						{
							westBlock = westData->GetBlock(CHUNK_SIZE - 1, y, z);
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

								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 3);
								mainIndices.push_back(currentVertex + 1);
								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 2);
								mainIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// East
					{
						int eastBlock;
						if (x < CHUNK_SIZE - 1)
						{
							eastBlock = chunkData->GetBlock(x + 1, y, z);
						}
						else
						{
							eastBlock = eastData->GetBlock(0, y, z);
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

								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 3);
								mainIndices.push_back(currentVertex + 1);
								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 2);
								mainIndices.push_back(currentVertex + 3);
								currentVertex += 4;
							}
						}
					}

					// Bottom
					{
						int bottomBlock;
						if (y > 0)
						{
							bottomBlock = chunkData->GetBlock(x, y - 1, z);
						}
						else
						{
							//int blockIndex = x * chunkSize * chunkSize + z * chunkSize + (chunkSize - 1);
							bottomBlock = downData->GetBlock(x, CHUNK_SIZE - 1, z);
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

								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 3);
								mainIndices.push_back(currentVertex + 1);
								mainIndices.push_back(currentVertex + 0);
								mainIndices.push_back(currentVertex + 2);
								mainIndices.push_back(currentVertex + 3);
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

							mainIndices.push_back(currentVertex + 0);
							mainIndices.push_back(currentVertex + 3);
							mainIndices.push_back(currentVertex + 1);
							mainIndices.push_back(currentVertex + 0);
							mainIndices.push_back(currentVertex + 2);
							mainIndices.push_back(currentVertex + 3);
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
			numTrianglesMain = mainIndices.size();

			glGenVertexArrays(1, &mainVAO);
			glGenBuffers(1, &mainVBO);
			glGenBuffers(1, &mainEBO);

			glBindVertexArray(mainVAO);

			glBindBuffer(GL_ARRAY_BUFFER, mainVBO);
			glBufferData(GL_ARRAY_BUFFER, mainVertices.size() * sizeof(Vertex), mainVertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mainIndices.size() * sizeof(unsigned int), mainIndices.data(), GL_STATIC_DRAW);

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
	mainShader->setMat4x4f( "model", model );


	glBindVertexArray(mainVAO);
	glDrawElements(GL_TRIANGLES, numTrianglesMain, GL_UNSIGNED_INT, 0);

	// Render billboard mesh
	billboardShader->use();
	billboardShader->setMat4x4f( "model", model );

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

	glBindVertexArray(waterVAO);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, worldPos);
	shader->setMat4x4f( "model", model );
	
	glDrawElements(GL_TRIANGLES, numTrianglesWater, GL_UNSIGNED_INT, 0);
}

uint16_t Chunk::GetBlockAtPos(int x, int y, int z)
{
	if (!ready)
		return 0;

	return chunkData->GetBlock(x, y, z);
}

void Chunk::UpdateBlock(int x, int y, int z, uint16_t newBlock)
{
	chunkData->SetBlock(x, y, z, newBlock);

	GenerateChunkMesh();

	// Main
	numTrianglesMain = mainIndices.size();

	glBindVertexArray(mainVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mainVBO);
	glBufferData(GL_ARRAY_BUFFER, mainVertices.size() * sizeof(Vertex), mainVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mainIndices.size() * sizeof(unsigned int), mainIndices.data(), GL_STATIC_DRAW);

	// Water
	numTrianglesWater = waterIndices.size();

	glBindVertexArray(waterVAO);

	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(WaterVertex), waterVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int), waterIndices.data(), GL_STATIC_DRAW);

	// Billboard
	numTrianglesBillboard = billboardIndices.size();;

	glBindVertexArray(billboardVAO);

	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, billboardVertices.size() * sizeof(BillboardVertex), billboardVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, billboardIndices.size() * sizeof(unsigned int), billboardIndices.data(), GL_STATIC_DRAW);


	if (x == 0)
	{
		Chunk* westChunk = Planet::planet->GetChunk({ chunkPos.x - 1, chunkPos.y, chunkPos.z });
		if (westChunk != nullptr)
			westChunk->UpdateChunk();
	}
	else if (x == CHUNK_SIZE - 1)
	{
		Chunk* eastChunk = Planet::planet->GetChunk({ chunkPos.x + 1, chunkPos.y, chunkPos.z });
		if (eastChunk != nullptr)
			eastChunk->UpdateChunk();
	}

	if (y == 0)
	{
		Chunk* downChunk = Planet::planet->GetChunk({ chunkPos.x, chunkPos.y - 1, chunkPos.z });
		if (downChunk != nullptr)
			downChunk->UpdateChunk();
	}
	else if (y == CHUNK_SIZE - 1)
	{
		Chunk* upChunk = Planet::planet->GetChunk({ chunkPos.x, chunkPos.y + 1, chunkPos.z });
		if (upChunk != nullptr)
			upChunk->UpdateChunk();
	}

	if (z == 0)
	{
		Chunk* northChunk = Planet::planet->GetChunk({ chunkPos.x, chunkPos.y, chunkPos.z - 1 });
		if (northChunk != nullptr)
			northChunk->UpdateChunk();
	}
	else if (z == CHUNK_SIZE - 1)
	{
		Chunk* southChunk = Planet::planet->GetChunk({ chunkPos.x, chunkPos.y, chunkPos.z + 1 });
		if (southChunk != nullptr)
			southChunk->UpdateChunk();
	}
}

void Chunk::UpdateChunk()
{
	GenerateChunkMesh();

	// Main
	numTrianglesMain = mainIndices.size();

	glBindVertexArray(mainVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mainVBO);
	glBufferData(GL_ARRAY_BUFFER, mainVertices.size() * sizeof(Vertex), mainVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mainIndices.size() * sizeof(unsigned int), mainIndices.data(), GL_STATIC_DRAW);

	// Water
	numTrianglesWater = waterIndices.size();

	glBindVertexArray(waterVAO);

	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(WaterVertex), waterVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int), waterIndices.data(), GL_STATIC_DRAW);

	// Billboard
	numTrianglesBillboard = billboardIndices.size();

	glBindVertexArray(billboardVAO);

	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, billboardVertices.size() * sizeof(BillboardVertex), billboardVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, billboardIndices.size() * sizeof(unsigned int), billboardIndices.data(), GL_STATIC_DRAW);
}