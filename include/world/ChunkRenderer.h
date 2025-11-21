#pragma once

#include <world/ChunkData.h>
#include <wv/core.h>

namespace ScuffedMinecraft
{
    struct ChunkVertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texPos;
    };

    class ChunkRenderer
    {
    public:
        ChunkRenderer(ChunkData* chunkData, const glm::ivec3& chunkId);
        ~ChunkRenderer();

        void SetNorthData(ChunkData* data) { m_northChunkData = data; }
        void SetSouthData(ChunkData* data) { m_southChunkData = data; }
        void SetEastData(ChunkData* data)  { m_eastChunkData  = data; }
        void SetWestData(ChunkData* data)  { m_westChunkData  = data; }
        void SetUpData(ChunkData* data)    { m_upChunkData    = data; }
        void SetDownData(ChunkData* data)  { m_downChunkData  = data; }

        void Render();

        void GenerateMesh();

        #ifdef DEBUG_MODE
        static float m_avgMeshGenTime;
        static int m_meshesGenerated;
        #endif

    private:
        ChunkData* m_chunkData;
        std::unique_ptr<WillowVox::VertexArrayObject> m_vao;

        ChunkData* m_northChunkData = nullptr;
        ChunkData* m_southChunkData = nullptr;
        ChunkData* m_eastChunkData = nullptr;
        ChunkData* m_westChunkData = nullptr;
        ChunkData* m_upChunkData = nullptr;
        ChunkData* m_downChunkData = nullptr;
        
        std::shared_ptr<WillowVox::Shader> m_chunkShader;

        std::vector<ChunkVertex> m_vertices;
        std::vector<int> m_indices;
        bool m_dirty = true;

        glm::ivec3 m_chunkId;
        glm::vec3 m_chunkPos;
    };
}