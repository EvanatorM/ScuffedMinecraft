#pragma once

#include <vector>
#include <thread>
#include <glm/glm.hpp>

struct Vertex
{
	char posX, posY, posZ;
	char texGridX, texGridY;
	char direction;

	Vertex(char _posX, char _posY, char _posZ, char _texGridX, char _texGridY, char _direction)
	{
		posX = _posX;
		posY = _posY;
		posZ = _posZ;

		texGridX = _texGridX;
		texGridY = _texGridY;

		direction = _direction;
	}
};

class Chunk
{
public:
	Chunk(unsigned int chunkSize, glm::vec3 chunkPos);
	~Chunk();

	void GenerateChunk();
	void Render(unsigned int modelLoc);

public:
	std::vector<unsigned int> chunkData;
	glm::vec3 chunkPos;
	bool ready;
	bool generated;

private:
	unsigned int vertexArrayObject;
	unsigned int vbo, ebo;
	unsigned int chunkSize;
	unsigned int numTriangles;
	glm::vec3 worldPos;
	std::thread chunkThread;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};