#pragma once

#include <world/WorldDefines.h>

namespace ScuffedMinecraft
{
	struct ChunkData
	{
	public:

		ChunkData()
		{
			for (int i = 0; i < CHUNK_VOLUME; i++)
				_data[i] = AIR;
		}

		~ChunkData()
		{

		}

		static constexpr bool InBounds(int x, int y, int z) noexcept
		{
			return 0 <= x && x < CHUNK_SIZE &&
				0 <= y && y < CHUNK_SIZE &&
				0 <= z && z < CHUNK_SIZE;
		}

		static constexpr int Index(int x, int y, int z) noexcept
		{
			return y + CHUNK_SIZE * (x + CHUNK_SIZE * z);
		}

		// 0 = invalid block position
		BlockId GetBlock(int x, int y, int z) const
		{
			if (!InBounds(x, y, z)) return 0;
			int index = Index(x, y, z);
			return _data[index];
		}

		// 0 = invalid block position
		BlockId GetBlock(int index) const
		{
			if (index < 0 || index > CHUNK_VOLUME)
				return 0;
			return _data[index];
		}

		// 0 = invalid block position
		BlockId SetBlock(int x, int y, int z, int blockId)
		{
			if (!InBounds(x, y, z)) return 0;
			int index = Index(x, y, z);
			_data[index] = blockId;
			return blockId;
		}

		// 0 = invalid block position
		BlockId SetBlock(int index, int blockId)
		{
			if (index < 0 || index > CHUNK_VOLUME)
				return 0;
			_data[index] = blockId;
			return blockId;
			
		}

	private:
		BlockId _data[CHUNK_VOLUME];
	};
}