#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/world/Chunk.h>
#include <WillowVox/world/ChunkData.h>
#include <WillowVox/math/ivec3Hash.h>
#include <WillowVox/rendering/Camera.h>
#include <WillowVox/world/WorldGen.h>
#include <WillowVox/resources/Block.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>

namespace WillowVox
{
    class WILLOWVOX_API ChunkManager
    {
    public:
        ChunkManager(WorldGen& worldGen) : _worldGen(worldGen) {}
        ~ChunkManager();
        void Start();
        void Update();
        void Render(Camera& camera);

        Chunk* GetChunk(int x, int y, int z);
        Chunk* GetChunk(glm::ivec3 pos);
        Chunk* GetChunkAtPos(float x, float y, float z);
        Chunk* GetChunkAtPos(glm::vec3 pos);

        uint16_t GetBlockIdAtPos(float x, float y, float z);
        uint16_t GetBlockIdAtPos(glm::vec3 pos);
        Block* GetBlockAtPos(float x, float y, float z);
        Block* GetBlockAtPos(glm::vec3 pos);

        void ClearChunkQueue();

        void SetPlayerObj(Camera* camera);

        int m_renderDistance = 10;
        int m_renderHeight = 2;

        // TEMP until asset manager
        BaseMaterial* m_solidMaterial;
        BaseMaterial* m_fluidMaterial;
        BaseMaterial* m_billboardMaterial;

    private:
        void ChunkThreadUpdate();

        WorldGen& _worldGen;

        std::unordered_map<glm::ivec3, Chunk*, ivec3Hash> _chunks;
        std::unordered_map<glm::ivec3, ChunkData*, ivec3Hash> _chunkData;
        std::queue<glm::ivec3> _chunkQueue; // For use only by chunk thread

        std::thread _chunkThread;
        std::mutex _chunkMutex;

        Camera* _playerObj;
        int _lastPlayerX = -100, _lastPlayerY = -100, _lastPlayerZ = -100;
        int _playerChunkX = -100, _playerChunkY = -100, _playerChunkZ = -100;

        bool _shouldEnd = false;
        bool _shouldClearChunkQueue = false;
    };
}