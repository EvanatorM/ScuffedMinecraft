#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/world/ChunkManager.h>
#include <WillowVox/world/Chunk.h>
#include <glm/glm.hpp>

#define RAY_STEP 0.01f

namespace WillowVox::Physics
{
    struct WILLOWVOX_API RaycastResult
    {
        bool m_hit;
        glm::vec3 m_hitPos;
        Chunk* m_chunk;
        int m_blockX;
        int m_blockY;
        int m_blockZ;
        int m_localBlockX;
        int m_localBlockY;
        int m_localBlockZ;

        RaycastResult(bool hit, glm::vec3 hitPos, Chunk* chunk, int blockX, int blockY, int blockZ, int localBlockX, int localBlockY, int localBlockZ)
            : m_hit(hit), m_hitPos(hitPos), m_chunk(chunk), m_blockX(blockX), m_blockY(blockY), m_blockZ(blockZ), m_localBlockX(localBlockX), m_localBlockY(localBlockY), m_localBlockZ(localBlockZ) {}
    };

    RaycastResult WILLOWVOX_API Raycast(ChunkManager& chunkManager, glm::vec3 startPos, glm::vec3 direction, float maxDistance);
}