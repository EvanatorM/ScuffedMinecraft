#include <world/ChunkManager.h>
#include <world/WorldGen.h>
#include <algorithm>
#include <chrono>

using namespace WillowVox;

namespace ScuffedMinecraft
{
    ChunkManager::ChunkManager(int worldSizeX, int worldMinY, int worldMaxY, int worldSizeZ)
        : m_worldSizeX(worldSizeX), m_worldMinY(worldMinY), m_worldMaxY(worldMaxY), m_worldSizeZ(worldSizeZ)
    {
        // Load assets
        auto& am = AssetManager::GetInstance();
        m_chunkShader = am.GetAsset<Shader>("chunk_shader");
        m_chunkTexture = am.GetAsset<Texture>("chunk_texture");

        // Start chunk thread
        m_chunkThread = std::thread(&ChunkManager::ChunkThread, this);
    }

    ChunkManager::~ChunkManager()
    {
        m_chunkThreadShouldStop = true;
        m_chunkThread.join();
    }

    void ChunkManager::SetBlockId(float x, float y, float z, BlockId blockId)
    {
        auto chunkId = WorldToChunkId(x, y, z);
        auto localPos = WorldToLocalChunkPos(x, y, z, chunkId);
        auto chunk = GetChunkData(chunkId);
        
        if (chunk && chunk->InBounds(localPos.x, localPos.y, localPos.z))
        {
            chunk->Set(localPos.x, localPos.y, localPos.z, blockId);

            {
                auto renderer = GetChunkRenderer(chunkId);
                if (renderer)
                    renderer->GenerateMesh();
            }
            
            // Remesh surrounding chunks if necessary
            if (localPos.x == 0)
            {
                auto renderer = GetChunkRenderer(chunkId.x - 1, chunkId.y, chunkId.z);
                if (renderer)
                    renderer->GenerateMesh();
            }
            else if (localPos.x == CHUNK_SIZE - 1)
            {
                auto renderer = GetChunkRenderer(chunkId.x + 1, chunkId.y, chunkId.z);
                if (renderer)
                    renderer->GenerateMesh();
            }
            if (localPos.y == 0)
            {
                auto renderer = GetChunkRenderer(chunkId.x, chunkId.y - 1, chunkId.z);
                if (renderer)
                    renderer->GenerateMesh();
            }
            else if (localPos.y == CHUNK_SIZE - 1)
            {
                auto renderer = GetChunkRenderer(chunkId.x, chunkId.y + 1, chunkId.z);
                if (renderer)
                    renderer->GenerateMesh();
            }
            if (localPos.z == 0)
            {
                auto renderer = GetChunkRenderer(chunkId.x, chunkId.y, chunkId.z - 1);
                if (renderer)
                    renderer->GenerateMesh();
            }
            else if (localPos.z == CHUNK_SIZE - 1)
            {
                auto renderer = GetChunkRenderer(chunkId.x, chunkId.y, chunkId.z + 1);
                if (renderer)
                    renderer->GenerateMesh();
            }
        }
    }
    
    ChunkData* ChunkManager::GetChunkData(int x, int y, int z)
    {
        return GetChunkData({ x, y, z });
    }

    ChunkData* ChunkManager::GetChunkDataAtPos(float x, float y, float z)
    {
        auto chunkId = WorldToChunkId(x, y, z);

        return GetChunkData(chunkId);
    }

    ChunkRenderer* ChunkManager::GetChunkRenderer(int x, int y, int z)
    {
        return GetChunkRenderer({ x, y, z });
    }

    ChunkRenderer* ChunkManager::GetChunkRendererAtPos(float x, float y, float z)
    {
        auto chunkId = WorldToChunkId(x, y, z);

        return GetChunkRenderer(chunkId);
    }

    ChunkData* ChunkManager::GetChunkData(const glm::ivec3& id)
    {
        std::lock_guard<std::mutex> chunkDataLock(m_chunkDataMutex);
        if (m_chunkData.find(id) != m_chunkData.end())
            return m_chunkData[id];

        return nullptr;
    }

    ChunkRenderer* ChunkManager::GetChunkRenderer(const glm::ivec3& id)
    {
        std::lock_guard<std::mutex> chunkRendererLock(m_chunkRendererMutex);
        if (m_chunkRenderers.find(id) != m_chunkRenderers.end())
            return m_chunkRenderers[id];

        return nullptr;
    }

    BlockId ChunkManager::GetBlockId(float x, float y, float z)
    {
        auto chunkId = WorldToChunkId(x, y, z);

        ChunkData* chunkData = GetChunkData(chunkId);
        if (!chunkData)
            return 0;

        auto localPos = WorldToLocalChunkPos(x, y, z, chunkId);

        return chunkData->Get(localPos.x, localPos.y, localPos.z);
    }

    void ChunkManager::Render()
    {
        {
            std::lock_guard<std::mutex> lock(m_chunkRendererDeletionMutex);
            while (!m_chunkRendererDeletionQueue.empty())
            {
                delete m_chunkRendererDeletionQueue.front();
                m_chunkRendererDeletionQueue.pop();
            }
        }

        m_chunkShader->Bind();
        m_chunkTexture->BindTexture(Texture::TEX00);
        {
            std::lock_guard<std::mutex> lock(m_chunkRendererMutex);
            for (auto [id, chunk] : m_chunkRenderers)
            {
                chunk->Render();
            }
        }
    }

    ChunkData* ChunkManager::GetOrGenerateChunkData(const glm::ivec3& id)
    {
        std::lock_guard<std::mutex> chunkDataLock(m_chunkDataMutex);
        if (m_chunkData.find(id) != m_chunkData.end())
        {
            return m_chunkData[id];
        }
        else if ((m_worldSizeX == 0 || (id.x >= -m_worldSizeX && id.x <= m_worldSizeX)) &&
                 (m_worldMinY  == 0 ||  id.y >= -m_worldMinY) && (m_worldMaxY == 0 || id.y <= m_worldMaxY) &&
                 (m_worldSizeZ == 0 || (id.z >= -m_worldSizeZ && id.z <= m_worldSizeZ)))
        {
            auto chunkPos = id * CHUNK_SIZE;

            #ifdef DEBUG_MODE
            auto start = std::chrono::high_resolution_clock::now();
            #endif

            auto data = new ChunkData();
            WorldGen::Generate(data, chunkPos);

            m_chunkData[id] = data;
            
            #ifdef DEBUG_MODE
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            m_chunkDataGenerated++;
            m_avgChunkDataGenTime = m_avgChunkDataGenTime + (duration.count() - m_avgChunkDataGenTime) / std::min(m_chunkDataGenerated, 1);
            #endif

            return data;
        }
        
        return nullptr;
    } 

    void ChunkManager::ChunkThread()
    {
        int prevXChunk = 1000;
        int prevYChunk = 1000;
        int prevZChunk = 1000;

        while (!m_chunkThreadShouldStop)
        {
            if (!m_camera) continue;

            int chunkX = m_camera->m_position.x < 0 ? (m_camera->m_position.x / CHUNK_SIZE) - 1 : m_camera->m_position.x / CHUNK_SIZE;
            int chunkY = m_camera->m_position.y < 0 ? (m_camera->m_position.y / CHUNK_SIZE) - 1 : m_camera->m_position.y / CHUNK_SIZE;
            int chunkZ = m_camera->m_position.z < 0 ? (m_camera->m_position.z / CHUNK_SIZE) - 1 : m_camera->m_position.z / CHUNK_SIZE;

            if (prevXChunk != chunkX || prevYChunk != chunkY || prevZChunk != chunkZ)
            {
                prevXChunk = chunkX;
                prevYChunk = chunkY;
                prevZChunk = chunkZ;

                // Clear queue
                std::queue<glm::ivec3> newQueue;
                std::swap(m_chunkQueue, newQueue);

                // Generate new queue
                m_chunkQueue.push({ chunkX, chunkY, chunkZ });

                for (int r = 0; r < m_renderDistance; r++)
                {
                    // Add middle chunks
                    for (int y = 0; y < m_renderHeight; y++)
                    {
                        m_chunkQueue.push({ chunkX,     chunkY + y, chunkZ + r });
                        m_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ });
                        m_chunkQueue.push({ chunkX,     chunkY + y, chunkZ - r });
                        m_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ });

                        if (y > 0)
                        {
                            m_chunkQueue.push({ chunkX,     chunkY - y, chunkZ + r });
                            m_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ });
                            m_chunkQueue.push({ chunkX,     chunkY - y, chunkZ - r });
                            m_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ });
                        }
                    }

                    // Add edges
                    for (int e = 1; e < r; e++)
                    {
                        for (int y = 0; y <= m_renderHeight; y++)
                        {
                            m_chunkQueue.push({ chunkX + e, chunkY + y, chunkZ + r });
                            m_chunkQueue.push({ chunkX - e, chunkY + y, chunkZ + r });

                            m_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ + e });
                            m_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ - e });

                            m_chunkQueue.push({ chunkX + e, chunkY + y, chunkZ - r });
                            m_chunkQueue.push({ chunkX - e, chunkY + y, chunkZ - r });

                            m_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ + e });
                            m_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ - e });

                            if (y > 0)
                            {
                                m_chunkQueue.push({ chunkX + e, chunkY - y, chunkZ + r });
                                m_chunkQueue.push({ chunkX - e, chunkY - y, chunkZ + r });

                                m_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ + e });
                                m_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ - e });

                                m_chunkQueue.push({ chunkX + e, chunkY - y, chunkZ - r });
                                m_chunkQueue.push({ chunkX - e, chunkY - y, chunkZ - r });

                                m_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ + e });
                                m_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ - e });
                            }
                        }
                    }

                    // Add corners
                    for (int y = 0; y <= m_renderHeight; y++)
                    {
                        m_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ + r });
                        m_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ - r });
                        m_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ + r });
                        m_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ - r });

                        if (y > 0)
                        {
                            m_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ + r });
                            m_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ - r });
                            m_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ + r });
                            m_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ - r });
                        }
                    }
                }

                // Delete chunk renderers out of range
                {
                    std::lock_guard<std::mutex> chunkRenderLock(m_chunkRendererMutex);
                    for (auto it = m_chunkRenderers.begin(); it != m_chunkRenderers.end();)
                    {
                        glm::ivec3 id = it->first;
                        if (std::abs(id.x - chunkX) > m_renderDistance ||
                            std::abs(id.y - chunkY) > m_renderHeight ||
                            std::abs(id.z - chunkZ) > m_renderDistance)
                        {
                            {
                                std::lock_guard<std::mutex> deleteLock(m_chunkRendererDeletionMutex);
                                m_chunkRendererDeletionQueue.push(it->second);
                            }
                            it = m_chunkRenderers.erase(it);
                        }
                        else
                            ++it;
                    }
                }

                // Delete chunk data out of range
                {
                    std::lock_guard<std::mutex> chunkDataLock(m_chunkDataMutex);
                    for (auto it = m_chunkData.begin(); it != m_chunkData.end();)
                    {
                        glm::ivec3 id = it->first;
                        std::lock_guard<std::mutex> chunkRenderLock(m_chunkRendererMutex);
                        if (m_chunkRenderers.find(id) == m_chunkRenderers.end() &&
                            m_chunkRenderers.find({ id.x + 1, id.y, id.z }) == m_chunkRenderers.end() &&
                            m_chunkRenderers.find({ id.x - 1, id.y, id.z }) == m_chunkRenderers.end() &&
                            m_chunkRenderers.find({ id.x, id.y + 1, id.z }) == m_chunkRenderers.end() &&
                            m_chunkRenderers.find({ id.x, id.y - 1, id.z }) == m_chunkRenderers.end() &&
                            m_chunkRenderers.find({ id.x, id.y, id.z + 1 }) == m_chunkRenderers.end() &&
                            m_chunkRenderers.find({ id.x, id.y, id.z - 1 }) == m_chunkRenderers.end())
                        {
                            delete it->second;
                            it = m_chunkData.erase(it);
                        }
                        else
                            ++it;
                    }
                }
            }

            if (!m_chunkQueue.empty())
            {
                auto id = m_chunkQueue.front();
                m_chunkQueue.pop();

                if (!((m_worldSizeX == 0 || (id.x >= -m_worldSizeX && id.x <= m_worldSizeX)) &&
                    (m_worldMinY == 0 || id.y >= -m_worldMinY) && (m_worldMaxY == 0 || id.y <= m_worldMaxY) &&
                    (m_worldSizeZ == 0 || (id.z >= -m_worldSizeZ && id.z <= m_worldSizeZ))))
                    continue;
                
                {
                    std::lock_guard<std::mutex> chunkRendererLock(m_chunkRendererMutex);
                    if (m_chunkRenderers.find(id) != m_chunkRenderers.end())
                        continue;
                }

                // Create chunk data
                ChunkData* data = GetOrGenerateChunkData(id);

                // Create chunk renderer
                auto chunk = new ChunkRenderer(m_chunkData[id], id);

                // Set neighboring chunks
                chunk->SetSouthData(GetOrGenerateChunkData({ id.x, id.y, id.z + 1 }));
                chunk->SetNorthData(GetOrGenerateChunkData({ id.x, id.y, id.z - 1 }));
                chunk->SetEastData(GetOrGenerateChunkData({  id.x + 1, id.y, id.z }));
                chunk->SetWestData(GetOrGenerateChunkData({  id.x - 1, id.y, id.z }));
                chunk->SetUpData(GetOrGenerateChunkData({    id.x, id.y + 1, id.z }));
                chunk->SetDownData(GetOrGenerateChunkData({  id.x, id.y - 1, id.z }));

                // Generate chunk mesh data
                chunk->GenerateMesh();

                // Add chunk to map
                std::lock_guard<std::mutex> lock(m_chunkRendererMutex);
                m_chunkRenderers[id] = chunk;
            }
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}