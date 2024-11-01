#include "Block.h"

Block::Block(char minX, char minY, char maxX, char maxY, BLOCK_TYPE blockType, std::string blockName)
	: blockType(blockType), blockName(blockName)
{
	topMinX = minX;
	topMinY = minY;
	topMaxX = maxX;
	topMaxY = maxY;

	bottomMinX = minX;
	bottomMinY = minY;
	bottomMaxX = maxX;
	bottomMaxY = maxY;

	sideMinX = minX;
	sideMinY = minY;
	sideMaxX = maxX;
	sideMaxY = maxY;
}

Block::Block(char topMinX, char topMinY, char topMaxX, char topMaxY,
	char bottomMinX, char bottomMinY, char bottomMaxX, char bottomMaxY,
	char sideMinX, char sideMinY, char sideMaxX, char sideMaxY, BLOCK_TYPE blockType, std::string blockName)
	: blockType(blockType), blockName(blockName)
{
	this->topMinX = topMinX;
	this->topMinY = topMinY;
	this->topMaxX = topMaxX;
	this->topMaxY = topMaxY;

	this->bottomMinX = bottomMinX;
	this->bottomMinY = bottomMinY;
	this->bottomMaxX = bottomMaxX;
	this->bottomMaxY = bottomMaxY;

	this->sideMinX = sideMinX;
	this->sideMinY = sideMinY;
	this->sideMaxX = sideMaxX;
	this->sideMaxY = sideMaxY;
}