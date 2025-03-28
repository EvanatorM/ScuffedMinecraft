#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/world/WorldGlobals.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace WillowVox
{
    struct WILLOWVOX_API ChunkData
    {
    public:
        ChunkData(uint16_t* voxels, glm::ivec3 offset) : m_voxels(voxels), m_offset(offset) {}
        ChunkData() : m_offset({ 0, 0, 0 }) { m_voxels = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE]; }
        ~ChunkData() { delete[] m_voxels; }

        inline int GetIndex(int x, int y, int z) const
        {
            return x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
        }

        uint16_t GetBlock(int x, int y, int z)
        {
            return m_voxels[GetIndex(x, y, z)];
        }

        void SetBlock(int x, int y, int z, uint16_t block)
        {
            m_voxels[GetIndex(x, y, z)] = block;
        }

        uint16_t* m_voxels;
        glm::ivec3 m_offset;
    };
}