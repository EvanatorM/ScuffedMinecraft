#pragma once

#include <world/ChunkRenderer.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <wv/core.h>
#include <thread>
#include <mutex>
#include <queue>

namespace ScuffedMinecraft
{
    class ChunkManager
    {
    public:
        ChunkManager(int worldSizeX = 0, int worldMinY = 0, int worldMaxY = 0, int worldSizeZ = 0);
        ~ChunkManager();

        ChunkData* GetChunkData(int x, int y, int z);
        ChunkData* GetChunkData(const glm::ivec3& id);
        ChunkData* GetChunkDataAtPos(float x, float y, float z);
        
        ChunkRenderer* GetChunkRenderer(int x, int y, int z);
        ChunkRenderer* GetChunkRenderer(const glm::ivec3& id);
        ChunkRenderer* GetChunkRendererAtPos(float x, float y, float z);
        
        BlockId GetBlockId(float x, float y, float z);

        void SetBlockId(float x, float y, float z, BlockId blockId);

        void Render();

        void SetCamera(WillowVox::Camera* camera) { m_camera = camera; }
        void SetRenderDistance(int renderDistance, int renderHeight) { m_renderDistance = renderDistance; m_renderHeight = renderHeight; }

        inline glm::ivec3 WorldToBlockPos(float x, float y, float z)
        {
            int blockX = x < 0 ? floor(x) : x;
            int blockY = y < 0 ? floor(y) : y;
            int blockZ = z < 0 ? floor(z) : z;

            return { blockX, blockY, blockZ };
        }

        inline glm::ivec3 WorldToChunkId(float x, float y, float z)
        {
            auto blockPos = WorldToBlockPos(x, y, z);

            int chunkX = blockPos.x < 0 ? floor(blockPos.x / (float)CHUNK_SIZE) : blockPos.x / CHUNK_SIZE;
            int chunkY = blockPos.y < 0 ? floor(blockPos.y / (float)CHUNK_SIZE) : blockPos.y / CHUNK_SIZE;
            int chunkZ = blockPos.z < 0 ? floor(blockPos.z / (float)CHUNK_SIZE) : blockPos.z / CHUNK_SIZE;

            return { chunkX, chunkY, chunkZ };
        }
        inline glm::ivec3 BlockToChunkId(int x, int y, int z)
        {
            int chunkX = x < 0 ? floor(x / (float)CHUNK_SIZE) : x / CHUNK_SIZE;
            int chunkY = y < 0 ? floor(y / (float)CHUNK_SIZE) : y / CHUNK_SIZE;
            int chunkZ = z < 0 ? floor(z / (float)CHUNK_SIZE) : z / CHUNK_SIZE;

            return { chunkX, chunkY, chunkZ };
        }

        inline glm::ivec3 WorldToLocalChunkPos(float x, float y, float z, const glm::ivec3& id)
        {
            auto blockPos = WorldToBlockPos(x, y, z);
            return { blockPos.x - (id.x * CHUNK_SIZE),
                     blockPos.y - (id.y * CHUNK_SIZE),
                     blockPos.z - (id.z * CHUNK_SIZE) };
        }
        inline glm::ivec3 BlockToLocalChunkPos(int x, int y, int z, const glm::ivec3& id)
        {
            return { x - (id.x * CHUNK_SIZE),
                     y - (id.y * CHUNK_SIZE),
                     z - (id.z * CHUNK_SIZE) };
        }

        #ifdef DEBUG_MODE
        float m_avgChunkDataGenTime = 0.0f;
        int m_chunkDataGenerated = 0;
        #endif

    private:
        ChunkData* GetOrGenerateChunkData(const glm::ivec3& id);
        void ChunkThread();

        WillowVox::Camera* m_camera = nullptr;
        int m_renderDistance, m_renderHeight;
        std::queue<glm::ivec3> m_chunkQueue;

        std::unordered_map<glm::ivec3, ChunkData*> m_chunkData;
        std::mutex m_chunkDataMutex;
        std::unordered_map<glm::ivec3, ChunkRenderer*> m_chunkRenderers;
        std::mutex m_chunkRendererMutex;

        std::queue<ChunkRenderer*> m_chunkRendererDeletionQueue;
        std::mutex m_chunkRendererDeletionMutex;

        int m_worldSizeX, m_worldMinY, m_worldMaxY, m_worldSizeZ;

        std::shared_ptr<WillowVox::Shader> m_chunkShader;
        std::shared_ptr<WillowVox::Texture> m_chunkTexture;

        std::thread m_chunkThread;
        bool m_chunkThreadShouldStop = false;
    };
}