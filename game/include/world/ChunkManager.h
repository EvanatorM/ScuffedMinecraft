#pragma once

#include <world/ChunkData.h>
#include <world/ChunkRenderer.h>
#include <world/ChunkRef.h>
#include <world/WorldGen.h>
#include <wv/core.h>
#include <wv/pch.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <condition_variable>

namespace ScuffedMinecraft
{
	class ChunkManager
	{
	public:
		ChunkManager(WillowVox::Shader* shader, WillowVox::Camera* camera);
		~ChunkManager();
		
		glm::ivec3 WorldToBlockPos(const glm::vec3& pos) const;
		glm::ivec3 WorldToBlockPos(float x, float y, float z) const;
		glm::ivec3 WorldToChunkId(const glm::vec3& pos) const;
		glm::ivec3 WorldToChunkId(float x, float y, float z) const;
		glm::ivec3 BlockToChunkId(const glm::ivec3& pos) const;
		glm::ivec3 BlockToChunkId(int x, int y, int z) const;

		ChunkRef<ChunkData> GetChunkDataFromId(const glm::ivec3& id);
		ChunkRef<ChunkData> GetChunkDataFromWorldPos(const glm::vec3& pos);
		ChunkRef<ChunkData> GetChunkDataFromWorldPos(float x, float y, float z);
		
		void Update();
		void Render();

		void ResetQueue();

		int GetChunkRenderersSize();

	private:
		ChunkRef<ChunkData> GetOrCreateChunkData(const glm::ivec3& id);

		void ChunkThreadUpdate();

		std::unordered_map<glm::ivec3, ChunkRefOwner<ChunkData>> _chunkData;
		std::shared_mutex _chunkDataMutex;
		std::unordered_map<glm::ivec3, ChunkRefOwner<ChunkRenderer>> _chunkRenderers;
		std::shared_mutex _chunkRenderersMutex;

		std::thread _chunkThread;
		bool _chunkThreadShouldEnd = false;
		std::mutex _chunkThreadMutex;
		std::condition_variable _chunkThreadCv;

		WillowVox::Shader* _shader;

		// Only to be accessed by chunk thread
		std::queue<glm::ivec3> _chunkQueue;
		bool _chunkQueueShouldReset = true;
		std::queue<glm::ivec3> _chunkRendererDeletionQueue;

		WillowVox::Camera* _camera;
		glm::ivec3 _lastCamChunk;
		glm::ivec3 _currentCamChunk;

		int _renderDistance = 5;
		int _renderHeight = 2;

		WorldGen _worldGen;
	};
}