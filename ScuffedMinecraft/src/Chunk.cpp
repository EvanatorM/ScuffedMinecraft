#include "Chunk.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Planet.h"
#include "Blocks.h"
#include "WorldGen.h"

Chunk::Chunk(unsigned int chunkSize, glm::vec3 chunkPos)
{
	this->chunkSize = chunkSize;
	this->chunkPos = chunkPos;
	worldPos = glm::vec3(chunkPos.x * chunkSize, chunkPos.y * chunkSize, chunkPos.z * chunkSize);

	ready = false;
	generated = false;
	chunkThread = std::thread(&Chunk::GenerateChunk, this);
}

Chunk::~Chunk()
{
	if (chunkThread.joinable())
		chunkThread.join();

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vertexArrayObject);
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
	//auto northData = Planet::planet->GetChunkData(chunkPos.x, chunkPos.y, chunkPos.z - 1);
	//auto southData = Planet::planet->GetChunkData(chunkPos.x, chunkPos.y, chunkPos.z + 1);
	//auto eastData = Planet::planet->GetChunkData(chunkPos.x + 1, chunkPos.y, chunkPos.z);
	//auto westData = Planet::planet->GetChunkData(chunkPos.x - 1, chunkPos.y, chunkPos.z);
	//auto upData = Planet::planet->GetChunkData(chunkPos.x, chunkPos.y + 1, chunkPos.z);
	//auto downData = Planet::planet->GetChunkData(chunkPos.x, chunkPos.y - 1, chunkPos.z);

	//std::cout << "Got chunk data in thread: " << std::this_thread::get_id() << '\n';

	unsigned int currentVertex = 0;
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

					if (northBlock == 0)
					{
						vertices.push_back(Vertex(x + 1, y + 0, z + 0, block->sideMinX, block->sideMinY));
						vertices.push_back(Vertex(x + 0, y + 0, z + 0, block->sideMaxX, block->sideMinY));
						vertices.push_back(Vertex(x + 1, y + 1, z + 0, block->sideMinX, block->sideMaxY));
						vertices.push_back(Vertex(x + 0, y + 1, z + 0, block->sideMaxX, block->sideMaxY));

						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 3);
						indices.push_back(currentVertex + 1);
						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 2);
						indices.push_back(currentVertex + 3);
						currentVertex += 4;
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
					if (southBlock == 0)
					{
						vertices.push_back(Vertex(x + 0, y + 0, z + 1, block->sideMinX, block->sideMinY));
						vertices.push_back(Vertex(x + 1, y + 0, z + 1, block->sideMaxX, block->sideMinY));
						vertices.push_back(Vertex(x + 0, y + 1, z + 1, block->sideMinX, block->sideMaxY));
						vertices.push_back(Vertex(x + 1, y + 1, z + 1, block->sideMaxX, block->sideMaxY));

						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 3);
						indices.push_back(currentVertex + 1);
						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 2);
						indices.push_back(currentVertex + 3);
						currentVertex += 4;
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
						int blockIndex = (chunkSize - 1) *chunkSize * chunkSize + z * chunkSize + y;
						westBlock = westData[blockIndex];
					}
					if (westBlock == 0)
					{
						vertices.push_back(Vertex(x + 0, y + 0, z + 0, block->sideMinX, block->sideMinY));
						vertices.push_back(Vertex(x + 0, y + 0, z + 1, block->sideMaxX, block->sideMinY));
						vertices.push_back(Vertex(x + 0, y + 1, z + 0, block->sideMinX, block->sideMaxY));
						vertices.push_back(Vertex(x + 0, y + 1, z + 1, block->sideMaxX, block->sideMaxY));

						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 3);
						indices.push_back(currentVertex + 1);
						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 2);
						indices.push_back(currentVertex + 3);
						currentVertex += 4;
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
					if (eastBlock == 0)
					{
						vertices.push_back(Vertex(x + 1, y + 0, z + 1, block->sideMinX, block->sideMinY));
						vertices.push_back(Vertex(x + 1, y + 0, z + 0, block->sideMaxX, block->sideMinY));
						vertices.push_back(Vertex(x + 1, y + 1, z + 1, block->sideMinX, block->sideMaxY));
						vertices.push_back(Vertex(x + 1, y + 1, z + 0, block->sideMaxX, block->sideMaxY));

						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 3);
						indices.push_back(currentVertex + 1);
						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 2);
						indices.push_back(currentVertex + 3);
						currentVertex += 4;
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
					if (bottomBlock == 0)
					{
						vertices.push_back(Vertex(x + 1, y + 0, z + 1, block->bottomMinX, block->bottomMinY));
						vertices.push_back(Vertex(x + 0, y + 0, z + 1, block->bottomMaxX, block->bottomMinY));
						vertices.push_back(Vertex(x + 1, y + 0, z + 0, block->bottomMinX, block->bottomMaxY));
						vertices.push_back(Vertex(x + 0, y + 0, z + 0, block->bottomMaxX, block->bottomMaxY));

						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 3);
						indices.push_back(currentVertex + 1);
						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 2);
						indices.push_back(currentVertex + 3);
						currentVertex += 4;
					}
				}

				// Top
				{
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
					if (topBlock == 0)
					{
						vertices.push_back(Vertex(x + 0, y + 1, z + 1, block->topMinX, block->topMinY));
						vertices.push_back(Vertex(x + 1, y + 1, z + 1, block->topMaxX, block->topMinY));
						vertices.push_back(Vertex(x + 0, y + 1, z + 0, block->topMinX, block->topMaxY));
						vertices.push_back(Vertex(x + 1, y + 1, z + 0, block->topMaxX, block->topMaxY));

						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 3);
						indices.push_back(currentVertex + 1);
						indices.push_back(currentVertex + 0);
						indices.push_back(currentVertex + 2);
						indices.push_back(currentVertex + 3);
						currentVertex += 4;
					}
				}
			}
		}
	}

	//std::cout << "Finished generating in thread: " << std::this_thread::get_id() << '\n';

	generated = true;

	//std::cout << "Generated: " << generated << '\n';
}

void Chunk::Render(unsigned int modelLoc)
{
	if (!ready)
	{
		if (generated)
		{
			numTriangles = indices.size();

			glGenVertexArrays(1, &vertexArrayObject);
			glBindVertexArray(vertexArrayObject);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, posX));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texGridX));
			glEnableVertexAttribArray(1);

			glGenBuffers(1, &ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
			ready = true;
		}

		return;
	}

	//std::cout << "Rendering chunk " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << '\n'
	//	<< "Chunk VAO: " << vertexArrayObject << '\n' << "Triangles: " << numTriangles << '\n';

	glBindVertexArray(vertexArrayObject);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, worldPos);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, numTriangles, GL_UNSIGNED_INT, 0);
}
