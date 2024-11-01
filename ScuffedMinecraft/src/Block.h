#pragma once

#include <string>

struct Block
{
public:
	enum BLOCK_TYPE
	{
		SOLID,
		TRANSPARENT,
		LEAVES,
		BILLBOARD,
		LIQUID
	};

	char topMinX, topMinY, topMaxX, topMaxY;
	char bottomMinX, bottomMinY, bottomMaxX, bottomMaxY;
	char sideMinX, sideMinY, sideMaxX, sideMaxY;
	BLOCK_TYPE blockType;
	std::string blockName;

	Block(char minX, char minY, char maxX, char maxY, BLOCK_TYPE blockType, std::string blockName);
	Block(char topMinX, char topMinY, char topMaxX, char topMaxY, 
		char bottomMinX, char bottomMinY, char bottomMaxX, char bottomMaxY,
		char sideMinX, char sideMinY, char sideMaxX, char sideMaxY, BLOCK_TYPE blockType, std::string blockName);

};