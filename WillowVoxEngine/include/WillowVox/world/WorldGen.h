#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/world/ChunkData.h>
#include <WillowVox/core/Logger.h>
#include <cstdint>

namespace WillowVox
{
    class WILLOWVOX_API WorldGen
    {
    public:
        WorldGen(int seed) : m_seed(seed) {}

        virtual void GenerateChunkData(ChunkData& chunkData)
        {
            int i = 0;
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                for (int y = 0; y < CHUNK_SIZE; y++)
                {
                    for (int z = 0; z < CHUNK_SIZE; z++)
                    {
                        chunkData.m_voxels[i] = GetBlock(x + chunkData.m_offset.x, y + chunkData.m_offset.y, z + chunkData.m_offset.z);
                        i++;
                    }
                }
            }
        }

        virtual uint16_t GetBlock(int x, int y, int z)
        {
            return 0;
        }

        int m_seed;
    };
}