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
					// billboardVertices.push_back may reallocate memory and there were 8 different calls to that function
					// Not only that increases the binary size, but that also increases the amount of memory reallocations
					// the insert function reallocates the memory only onces and reduces the binary size significantly, as it's only 1 call
					billboardVertices.insert(billboardVertices.end(), std::initializer_list<BillboardVertex>{
						BillboardVertex(x + .85355f, y + 0, z + .85355f, block->sideMinX, block->sideMinY),
						BillboardVertex(x + .14645f, y + 0, z + .14645f, block->sideMaxX, block->sideMinY),
						BillboardVertex(x + .85355f, y + 1, z + .85355f, block->sideMinX, block->sideMaxY),
						BillboardVertex(x + .14645f, y + 1, z + .14645f, block->sideMaxX, block->sideMaxY),
						BillboardVertex(x + .14645f, y + 0, z + .85355f, block->sideMinX, block->sideMinY),
						BillboardVertex(x + .85355f, y + 0, z + .14645f, block->sideMaxX, block->sideMinY),
						BillboardVertex(x + .14645f, y + 1, z + .85355f, block->sideMinX, block->sideMaxY),
						BillboardVertex(x + .85355f, y + 1, z + .14645f, block->sideMaxX, block->sideMaxY)
					});

					billboardIndices.insert(billboardIndices.end(), std::initializer_list<std::uint32_t>{
						currentBillboardVertex + 0,
						currentBillboardVertex + 3,
						currentBillboardVertex + 1,
						currentBillboardVertex + 0,
						currentBillboardVertex + 2,
						currentBillboardVertex + 3,

						currentBillboardVertex + 4 + 0,
						currentBillboardVertex + 4 + 3,
						currentBillboardVertex + 4 + 1,
						currentBillboardVertex + 4 + 0,
						currentBillboardVertex + 4 + 2,
						currentBillboardVertex + 4 + 3
					});

					currentBillboardVertex += 8;
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
								waterVertices.insert(waterVertices.end(), std::initializer_list<WaterVertex>{
									WaterVertex(x + 1, y + 0, z + 0, block->sideMinX, block->sideMinY, 0, 0),
									WaterVertex(x + 0, y + 0, z + 0, block->sideMaxX, block->sideMinY, 0, 0),
									WaterVertex(x + 1, y + 1, z + 0, block->sideMinX, block->sideMaxY, 0, waterTopValue),
									WaterVertex(x + 0, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 0, waterTopValue)
								});

								waterIndices.insert(waterIndices.end(), std::initializer_list<std::uint32_t>{
									currentLiquidVertex + 0,
									currentLiquidVertex + 3,
									currentLiquidVertex + 1,
									currentLiquidVertex + 0,
									currentLiquidVertex + 2,
									currentLiquidVertex + 3
								});

								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.insert(mainVertices.end(), std::initializer_list<Vertex>{
									Vertex(x + 1, y + 0, z + 0, block->sideMinX, block->sideMinY, 0),
									Vertex(x + 0, y + 0, z + 0, block->sideMaxX, block->sideMinY, 0),
									Vertex(x + 1, y + 1, z + 0, block->sideMinX, block->sideMaxY, 0),
									Vertex(x + 0, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 0)
								});

								mainIndices.insert(mainIndices.end(), std::initializer_list<std::uint32_t>{
									currentVertex + 0,
									currentVertex + 3,
									currentVertex + 1,
									currentVertex + 0,
									currentVertex + 2,
									currentVertex + 3
								});

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
								waterVertices.insert(waterVertices.end(), std::initializer_list<WaterVertex>{
									WaterVertex(x + 0, y + 0, z + 1, block->sideMinX, block->sideMinY, 1, 0),
									WaterVertex(x + 1, y + 0, z + 1, block->sideMaxX, block->sideMinY, 1, 0),
									WaterVertex(x + 0, y + 1, z + 1, block->sideMinX, block->sideMaxY, 1, waterTopValue),
									WaterVertex(x + 1, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 1, waterTopValue)
								});

								waterIndices.insert(waterIndices.end(), std::initializer_list<std::uint32_t>{
									currentLiquidVertex + 0,
									currentLiquidVertex + 3,
									currentLiquidVertex + 1,
									currentLiquidVertex + 0,
									currentLiquidVertex + 2,
									currentLiquidVertex + 3
								});

								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.insert(mainVertices.end(), std::initializer_list<Vertex>{
									Vertex(x + 0, y + 0, z + 1, block->sideMinX, block->sideMinY, 1),
									Vertex(x + 1, y + 0, z + 1, block->sideMaxX, block->sideMinY, 1),
									Vertex(x + 0, y + 1, z + 1, block->sideMinX, block->sideMaxY, 1),
									Vertex(x + 1, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 1)
								});

								mainIndices.insert(mainIndices.end(), std::initializer_list<std::uint32_t>{
									currentVertex + 0,
									currentVertex + 3,
									currentVertex + 1,
									currentVertex + 0,
									currentVertex + 2,
									currentVertex + 3
								});

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
								waterVertices.insert(waterVertices.end(), std::initializer_list<WaterVertex>{
									WaterVertex(x + 0, y + 0, z + 0, block->sideMinX, block->sideMinY, 2, 0),
									WaterVertex(x + 0, y + 0, z + 1, block->sideMaxX, block->sideMinY, 2, 0),
									WaterVertex(x + 0, y + 1, z + 0, block->sideMinX, block->sideMaxY, 2, waterTopValue),
									WaterVertex(x + 0, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 2, waterTopValue)
								});

								waterIndices.insert(waterIndices.end(), std::initializer_list<std::uint32_t>{
									currentLiquidVertex + 0,
									currentLiquidVertex + 3,
									currentLiquidVertex + 1,
									currentLiquidVertex + 0,
									currentLiquidVertex + 2,
									currentLiquidVertex + 3
								});

								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.insert(mainVertices.end(), std::initializer_list<Vertex>{
									Vertex(x + 0, y + 0, z + 0, block->sideMinX, block->sideMinY, 2),
									Vertex(x + 0, y + 0, z + 1, block->sideMaxX, block->sideMinY, 2),
									Vertex(x + 0, y + 1, z + 0, block->sideMinX, block->sideMaxY, 2),
									Vertex(x + 0, y + 1, z + 1, block->sideMaxX, block->sideMaxY, 2)
								});

								mainIndices.insert(mainIndices.end(), std::initializer_list<std::uint32_t>{
									currentVertex + 0,
									currentVertex + 3,
									currentVertex + 1,
									currentVertex + 0,
									currentVertex + 2,
									currentVertex + 3
								});

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
								waterVertices.insert(waterVertices.end(), std::initializer_list<WaterVertex>{
									WaterVertex(x + 1, y + 0, z + 1, block->sideMinX, block->sideMinY, 3, 0),
									WaterVertex(x + 1, y + 0, z + 0, block->sideMaxX, block->sideMinY, 3, 0),
									WaterVertex(x + 1, y + 1, z + 1, block->sideMinX, block->sideMaxY, 3, waterTopValue),
									WaterVertex(x + 1, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 3, waterTopValue)
								});

								waterIndices.insert(waterIndices.end(), std::initializer_list<std::uint32_t>{
									currentLiquidVertex + 0,
									currentLiquidVertex + 3,
									currentLiquidVertex + 1,
									currentLiquidVertex + 0,
									currentLiquidVertex + 2,
									currentLiquidVertex + 3
								});

								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.insert(mainVertices.end(), std::initializer_list<Vertex>{
									Vertex(x + 1, y + 0, z + 1, block->sideMinX, block->sideMinY, 3),
									Vertex(x + 1, y + 0, z + 0, block->sideMaxX, block->sideMinY, 3),
									Vertex(x + 1, y + 1, z + 1, block->sideMinX, block->sideMaxY, 3),
									Vertex(x + 1, y + 1, z + 0, block->sideMaxX, block->sideMaxY, 3)
								});

								mainIndices.insert(mainIndices.end(), std::initializer_list<std::uint32_t>{
									currentVertex + 0,
									currentVertex + 3,
									currentVertex + 1,
									currentVertex + 0,
									currentVertex + 2,
									currentVertex + 3
								});

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
								waterVertices.insert(waterVertices.end(), std::initializer_list<WaterVertex>{
									WaterVertex(x + 1, y + 0, z + 1, block->bottomMinX, block->bottomMinY, 4, 0),
									WaterVertex(x + 0, y + 0, z + 1, block->bottomMaxX, block->bottomMinY, 4, 0),
									WaterVertex(x + 1, y + 0, z + 0, block->bottomMinX, block->bottomMaxY, 4, 0),
									WaterVertex(x + 0, y + 0, z + 0, block->bottomMaxX, block->bottomMaxY, 4, 0)
								});

								waterIndices.insert(waterIndices.end(), std::initializer_list<std::uint32_t>{
									currentLiquidVertex + 0,
									currentLiquidVertex + 3,
									currentLiquidVertex + 1,
									currentLiquidVertex + 0,
									currentLiquidVertex + 2,
									currentLiquidVertex + 3
								});

								currentLiquidVertex += 4;
							}
							else
							{
								mainVertices.insert(mainVertices.end(), std::initializer_list<Vertex>{
									Vertex(x + 1, y + 0, z + 1, block->bottomMinX, block->bottomMinY, 4),
									Vertex(x + 0, y + 0, z + 1, block->bottomMaxX, block->bottomMinY, 4),
									Vertex(x + 1, y + 0, z + 0, block->bottomMinX, block->bottomMaxY, 4),
									Vertex(x + 0, y + 0, z + 0, block->bottomMaxX, block->bottomMaxY, 4)
								});

								mainIndices.insert(mainIndices.end(), std::initializer_list<std::uint32_t>{
									currentVertex + 0,
									currentVertex + 3,
									currentVertex + 1,
									currentVertex + 0,
									currentVertex + 2,
									currentVertex + 3
								});

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
								waterVertices.insert(waterVertices.end(), std::initializer_list<WaterVertex>{
									WaterVertex(x + 0, y + 1, z + 1, block->topMinX, block->topMinY, 5, 1),
									WaterVertex(x + 1, y + 1, z + 1, block->topMaxX, block->topMinY, 5, 1),
									WaterVertex(x + 0, y + 1, z + 0, block->topMinX, block->topMaxY, 5, 1),
									WaterVertex(x + 1, y + 1, z + 0, block->topMaxX, block->topMaxY, 5, 1),
									WaterVertex(x + 1, y + 1, z + 1, block->topMinX, block->topMinY, 5, 1),
									WaterVertex(x + 0, y + 1, z + 1, block->topMaxX, block->topMinY, 5, 1),
									WaterVertex(x + 1, y + 1, z + 0, block->topMinX, block->topMaxY, 5, 1),
									WaterVertex(x + 0, y + 1, z + 0, block->topMaxX, block->topMaxY, 5, 1)
								});

								waterIndices.insert(waterIndices.end(), std::initializer_list<std::uint32_t>{
									currentLiquidVertex + 0,
									currentLiquidVertex + 3,
									currentLiquidVertex + 1,
									currentLiquidVertex + 0,
									currentLiquidVertex + 2,
									currentLiquidVertex + 3,

									currentLiquidVertex + 4 + 0,
									currentLiquidVertex + 4 + 3,
									currentLiquidVertex + 4 + 1,
									currentLiquidVertex + 4 + 0,
									currentLiquidVertex + 4 + 2,
									currentLiquidVertex + 4 + 3
								});

								currentLiquidVertex += 8;
							}
						}
						else if (topBlockType->blockType == Block::LEAVES
							|| topBlockType->blockType == Block::TRANSPARENT
							|| topBlockType->blockType == Block::BILLBOARD
							|| topBlockType->blockType == Block::LIQUID)
						{
							mainVertices.insert(mainVertices.end(), std::initializer_list<Vertex>{
								Vertex(x + 0, y + 1, z + 1, block->topMinX, block->topMinY, 5),
								Vertex(x + 1, y + 1, z + 1, block->topMaxX, block->topMinY, 5),
								Vertex(x + 0, y + 1, z + 0, block->topMinX, block->topMaxY, 5),
								Vertex(x + 1, y + 1, z + 0, block->topMaxX, block->topMaxY, 5)
							});

							mainIndices.insert(mainIndices.end(), std::initializer_list<std::uint32_t>{
								currentVertex + 0,
								currentVertex + 3,
								currentVertex + 1,
								currentVertex + 0,
								currentVertex + 2,
								currentVertex + 3
							});

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