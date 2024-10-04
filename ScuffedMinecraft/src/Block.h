#pragma once

struct Block
{
public:
	char topMinX, topMinY, topMaxX, topMaxY;
	char bottomMinX, bottomMinY, bottomMaxX, bottomMaxY;
	char sideMinX, sideMinY, sideMaxX, sideMaxY;
	bool transparent;
	bool billboard;

	Block(char minX, char minY, char maxX, char maxY, bool transparent = false, bool billboard = false);
	Block(char topMinX, char topMinY, char topMaxX, char topMaxY, 
		char bottomMinX, char bottomMinY, char bottomMaxX, char bottomMaxY,
		char sideMinX, char sideMinY, char sideMaxX, char sideMaxY, bool transparent = false, bool billboard = false);
};