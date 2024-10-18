#pragma once

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

struct WaterVertex
{
	char posX, posY, posZ;
	char texGridX, texGridY;
	char direction;
	char top;

	WaterVertex(char _posX, char _posY, char _posZ, char _texGridX, char _texGridY, char _direction, char _top)
	{
		posX = _posX;
		posY = _posY;
		posZ = _posZ;

		texGridX = _texGridX;
		texGridY = _texGridY;

		direction = _direction;

		top = _top;
	}
};

struct BillboardVertex
{
	float posX, posY, posZ;
	char texGridX, texGridY;

	BillboardVertex(float _posX, float _posY, float _posZ, char _texGridX, char _texGridY)
	{
		posX = _posX;
		posY = _posY;
		posZ = _posZ;

		texGridX = _texGridX;
		texGridY = _texGridY;
	}
};