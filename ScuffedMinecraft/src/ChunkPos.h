#pragma once

struct ChunkPos
{
	int x;
	int y;
	int z;

	ChunkPos();
	ChunkPos(int x, int y, int z);

	bool operator==(const ChunkPos& other) const;
};