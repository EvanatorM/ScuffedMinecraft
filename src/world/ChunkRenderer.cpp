#include <world/ChunkRenderer.h>
#include <world/BlockRegistry.h>
#include <chrono>

using namespace WillowVox;

namespace ScuffedMinecraft
{
    #ifdef DEBUG_MODE
    float ChunkRenderer::m_avgMeshGenTime = 0;
    int ChunkRenderer::m_meshesGenerated = 0;
    #endif

    ChunkRenderer::ChunkRenderer(ChunkData* chunkData, const glm::ivec3& chunkId)
        : m_chunkData(chunkData), m_chunkId(chunkId), m_chunkPos(chunkId * CHUNK_SIZE)
    {
        auto& am = AssetManager::GetInstance();
        m_chunkShader = am.GetAsset<Shader>("chunk_shader");
    }

    ChunkRenderer::~ChunkRenderer()
    {
        //Logger::Log("Destroying ChunkRenderer at (%d, %d, %d)", m_chunkId.x, m_chunkId.y, m_chunkId.z);
    }

    void ChunkRenderer::Render()
    {
        // Create vao if it doesn't exist
        if (!m_vao)
        {
            m_vao = std::make_unique<VertexArrayObject>();
            m_vao->SetAttribPointer(0, 3, VertexBufferAttribType::FLOAT32, false, sizeof(ChunkVertex), offsetof(ChunkVertex, pos));
            m_vao->SetAttribPointer(1, 3, VertexBufferAttribType::FLOAT32, false, sizeof(ChunkVertex), offsetof(ChunkVertex, normal));
            m_vao->SetAttribPointer(2, 2, VertexBufferAttribType::FLOAT32, false, sizeof(ChunkVertex), offsetof(ChunkVertex, texPos));
        }

        // Buffer data is dirty
        if (m_dirty)
        {
            m_vao->BufferVertexData(m_vertices.size() * sizeof(ChunkVertex), m_vertices.data());
            m_vao->BufferElementData(ElementBufferAttribType::UINT32, m_indices.size(), m_indices.data());
            m_dirty = false;
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_chunkPos);
        m_chunkShader->SetMat4("model", model);

        m_vao->Draw();
    }

    inline void AddIndices(std::vector<int>& indices, int& vertexCount)
    {
        indices.push_back(vertexCount + 0);
        indices.push_back(vertexCount + 2);
        indices.push_back(vertexCount + 1);

        indices.push_back(vertexCount + 1);
        indices.push_back(vertexCount + 2);
        indices.push_back(vertexCount + 3);
        vertexCount += 4;
    }

    void ChunkRenderer::GenerateMesh()
    {
        #ifdef DEBUG_MODE
        auto start = std::chrono::high_resolution_clock::now();
        #endif

        m_vertices.clear();
        m_indices.clear();
        int vertexCount = 0;

        auto& blockRegistry = BlockRegistry::GetInstance();

        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                for (int y = 0; y < CHUNK_SIZE; y++)
                {
                    BlockId id = m_chunkData->Get(x, y, z);
                    if (id == 0)
                        continue;

                    auto& block = blockRegistry.GetBlock(id);

                    // South Face
                    {
                        bool south;
                        if (z + 1 >= CHUNK_SIZE)
                        {
                            if (m_southChunkData)
                                south = m_southChunkData->Get(x, y, 0) == 0;
                            else
                                south = true;
                        }
                        else
                            south = m_chunkData->Get(x, y, z + 1) == 0;
                        if (south)
                        {
                            // South Face
                            m_vertices.push_back({ { x + 0, y + 0, z + 1 }, { 0, 0, 1 }, { block.sideTexMinX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 1, y + 0, z + 1 }, { 0, 0, 1 }, { block.sideTexMaxX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 0, y + 1, z + 1 }, { 0, 0, 1 }, { block.sideTexMinX, block.sideTexMaxY } });
                            m_vertices.push_back({ { x + 1, y + 1, z + 1 }, { 0, 0, 1 }, { block.sideTexMaxX, block.sideTexMaxY } });

                            AddIndices(m_indices, vertexCount);
                        }
                    }
                    
                    // North Face
                    {
                        bool north;
                        if (z < 1)
                        {
                            if (m_northChunkData)
                                north = m_northChunkData->Get(x, y, CHUNK_SIZE - 1) == 0;
                            else
                                north = true;
                        }
                        else
                            north = m_chunkData->Get(x, y, z - 1) == 0;
                        if (north)
                        {
                            // North Face
                            m_vertices.push_back({ { x + 1, y + 0, z + 0 }, { 0, 0, -1 }, { block.sideTexMinX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 0, y + 0, z + 0 }, { 0, 0, -1 }, { block.sideTexMaxX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 1, y + 1, z + 0 }, { 0, 0, -1 }, { block.sideTexMinX, block.sideTexMaxY } });
                            m_vertices.push_back({ { x + 0, y + 1, z + 0 }, { 0, 0, -1 }, { block.sideTexMaxX, block.sideTexMaxY } });

                            AddIndices(m_indices, vertexCount);
                        }
                    }
                    // East Face
                    {
                        bool east;
                        if (x + 1 >= CHUNK_SIZE)
                        {
                            if (m_eastChunkData)
                                east = m_eastChunkData->Get(0, y, z) == 0;
                            else
                                east = true;
                        }
                        else
                            east = m_chunkData->Get(x + 1, y, z) == 0;
                        if (east)
                        {
                            // East Face
                            m_vertices.push_back({ { x + 1, y + 0, z + 1 }, { -1, 0, 0 }, { block.sideTexMinX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 1, y + 0, z + 0 }, { -1, 0, 0 }, { block.sideTexMaxX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 1, y + 1, z + 1 }, { -1, 0, 0 }, { block.sideTexMinX, block.sideTexMaxY } });
                            m_vertices.push_back({ { x + 1, y + 1, z + 0 }, { -1, 0, 0 }, { block.sideTexMaxX, block.sideTexMaxY } });

                            AddIndices(m_indices, vertexCount);
                        }
                    }
                    // West Face
                    {
                        bool west;
                        if (x < 1)
                        {
                            if (m_westChunkData)
                                west = m_westChunkData->Get(CHUNK_SIZE - 1, y, z) == 0;
                            else
                                west = true;
                        }
                        else
                            west = m_chunkData->Get(x - 1, y, z) == 0;
                        if (west)
                        {
                            // West Face
                            m_vertices.push_back({ { x + 0, y + 0, z + 0 }, { 1, 0, 0 }, { block.sideTexMinX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 0, y + 0, z + 1 }, { 1, 0, 0 }, { block.sideTexMaxX, block.sideTexMinY } });
                            m_vertices.push_back({ { x + 0, y + 1, z + 0 }, { 1, 0, 0 }, { block.sideTexMinX, block.sideTexMaxY } });
                            m_vertices.push_back({ { x + 0, y + 1, z + 1 }, { 1, 0, 0 }, { block.sideTexMaxX, block.sideTexMaxY } });

                            AddIndices(m_indices, vertexCount);
                        }
                    }
                    // Up Face
                    {
                        bool up;
                        if (y + 1 >= CHUNK_SIZE)
                        {
                            if (m_upChunkData)
                                up = m_upChunkData->Get(x, 0, z) == 0;
                            else
                                up = true;
                        }
                        else
                            up = m_chunkData->Get(x, y + 1, z) == 0;
                        if (up)
                        {
                            // Up Face
                            m_vertices.push_back({ { x + 0, y + 1, z + 1 }, { 0, 1, 0 }, { block.topTexMinX, block.topTexMinY } });
                            m_vertices.push_back({ { x + 1, y + 1, z + 1 }, { 0, 1, 0 }, { block.topTexMaxX, block.topTexMinY } });
                            m_vertices.push_back({ { x + 0, y + 1, z + 0 }, { 0, 1, 0 }, { block.topTexMinX, block.topTexMaxY } });
                            m_vertices.push_back({ { x + 1, y + 1, z + 0 }, { 0, 1, 0 }, { block.topTexMaxX, block.topTexMaxY } });

                            AddIndices(m_indices, vertexCount);
                        }
                    }
                    // Down Face
                    {
                        bool down;
                        if (y < 1)
                        {
                            if (m_downChunkData)
                                down = m_downChunkData->Get(x, CHUNK_SIZE - 1, z) == 0;
                            else
                                down = true;
                        }
                        else
                            down = m_chunkData->Get(x, y - 1, z) == 0;
                        if (down)
                        {
                            // Down Face
                            m_vertices.push_back({ { x + 1, y + 0, z + 1 }, { 0, -1, 0 }, { block.bottomTexMinX, block.bottomTexMinY } });
                            m_vertices.push_back({ { x + 0, y + 0, z + 1 }, { 0, -1, 0 }, { block.bottomTexMaxX, block.bottomTexMinY } });
                            m_vertices.push_back({ { x + 1, y + 0, z + 0 }, { 0, -1, 0 }, { block.bottomTexMinX, block.bottomTexMaxY } });
                            m_vertices.push_back({ { x + 0, y + 0, z + 0 }, { 0, -1, 0 }, { block.bottomTexMaxX, block.bottomTexMaxY } });

                            AddIndices(m_indices, vertexCount);
                        }
                    }
                }
            }
        }

        m_dirty = true;

        #ifdef DEBUG_MODE
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        m_meshesGenerated++;
        m_avgMeshGenTime = m_avgMeshGenTime + (duration.count() - m_avgMeshGenTime) / std::min(m_meshesGenerated, 1);
        #endif
    }
}