#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/MeshRenderer.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <WillowVox/world/ChunkData.h>
#include <WillowVox/rendering/engine-default/ChunkVertex.h>
#include <WillowVox/rendering/engine-default/FluidVertex.h>
#include <WillowVox/rendering/engine-default/Vertex.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>


namespace WillowVox
{
    class ChunkManager;
    
    class WILLOWVOX_API Chunk
    {
    public:
        Chunk(ChunkManager& chunkManager, BaseMaterial* solidMaterial, BaseMaterial* fluidMaterial, BaseMaterial* billboardMaterial, const glm::ivec3& chunkPos, const glm::vec3& worldPos);
        ~Chunk();

        void GenerateChunkMeshData();
        void GenerateChunkMesh();
        void RenderSolid(const glm::mat4& view, const glm::mat4& projection);
        void RenderTransparent();

        uint16_t GetBlockIdAtPos(int x, int y, int z);
        void SetBlock(int x, int y, int z, uint16_t block);
        void ReloadChunk();

        ChunkData* m_chunkData;
        ChunkData* m_northData;
        ChunkData* m_southData;
        ChunkData* m_eastData;
        ChunkData* m_westData;
        ChunkData* m_upData;
        ChunkData* m_downData;

        glm::ivec3 m_chunkPos;
        bool m_ready = false;

    private:
        ChunkManager& _chunkManager;

        glm::vec3 _worldPos;

        MeshRenderer* _solidMesh;
        MeshRenderer* _fluidMesh;
        MeshRenderer* _billboardMesh;
        BaseMaterial* _solidMaterial;
        BaseMaterial* _fluidMaterial;
        BaseMaterial* _billboardMaterial;

        std::vector<ChunkVertex> _solidVertices;
        std::vector<uint32_t> _solidIndices;
        std::vector<FluidVertex> _fluidVertices;
        std::vector<uint32_t> _fluidIndices;
        std::vector<Vertex> _billboardVertices;
        std::vector<uint32_t> _billboardIndices;
    };
}