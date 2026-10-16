#include <world/ChunkManager.h>
#include <algorithm>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	ChunkManager::ChunkManager(Shader* shader, Camera* camera)
		: _shader(shader), _camera(camera), _worldGen(0.0f, 10.0f, 100.0f, 5, 0.8f, 4.0f)
	{
		_chunkThread = std::thread(&ChunkManager::ChunkThreadUpdate, this);
	}

	ChunkManager::~ChunkManager()
	{
		_chunkThreadShouldEnd = true;
		_chunkThreadCv.notify_one();
		_chunkThread.join();
	}

	glm::ivec3 ChunkManager::WorldToBlockPos(const glm::vec3& pos) const
	{
		int blockX = pos.x < 0 ? std::floor(pos.x) : pos.x;
		int blockY = pos.y < 0 ? std::floor(pos.y) : pos.y;
		int blockZ = pos.z < 0 ? std::floor(pos.z) : pos.z;

		return { blockX, blockY, blockZ };
	}

	glm::ivec3 ChunkManager::WorldToBlockPos(float x, float y, float z) const
	{
		int blockX = x < 0 ? std::floor(x) : x;
		int blockY = y < 0 ? std::floor(y) : y;
		int blockZ = z < 0 ? std::floor(z) : z;

		return { blockX, blockY, blockZ };
	}

	glm::ivec3 ChunkManager::WorldToChunkId(const glm::vec3& pos) const
	{
		auto blockPos = WorldToBlockPos(pos);

		int chunkX = blockPos.x < 0 ? std::floor(blockPos.x / (float)CHUNK_SIZE) : blockPos.x / (float)CHUNK_SIZE;
		int chunkY = blockPos.y < 0 ? std::floor(blockPos.y / (float)CHUNK_SIZE) : blockPos.y / (float)CHUNK_SIZE;
		int chunkZ = blockPos.z < 0 ? std::floor(blockPos.z / (float)CHUNK_SIZE) : blockPos.z / (float)CHUNK_SIZE;

		return { chunkX, chunkY, chunkZ };
	}

	glm::ivec3 ChunkManager::WorldToChunkId(float x, float y, float z) const
	{
		auto blockPos = WorldToBlockPos(x, y, z);

		int chunkX = blockPos.x < 0 ? std::floor(blockPos.x / (float)CHUNK_SIZE) : blockPos.x / (float)CHUNK_SIZE;
		int chunkY = blockPos.y < 0 ? std::floor(blockPos.y / (float)CHUNK_SIZE) : blockPos.y / (float)CHUNK_SIZE;
		int chunkZ = blockPos.z < 0 ? std::floor(blockPos.z / (float)CHUNK_SIZE) : blockPos.z / (float)CHUNK_SIZE;

		return { chunkX, chunkY, chunkZ };
	}

	glm::ivec3 ChunkManager::BlockToChunkId(const glm::ivec3& pos) const
	{
		int chunkX = pos.x < 0 ? std::floor(pos.x / (float)CHUNK_SIZE) : pos.x / (float)CHUNK_SIZE;
		int chunkY = pos.y < 0 ? std::floor(pos.y / (float)CHUNK_SIZE) : pos.y / (float)CHUNK_SIZE;
		int chunkZ = pos.z < 0 ? std::floor(pos.z / (float)CHUNK_SIZE) : pos.z / (float)CHUNK_SIZE;

		return { chunkX, chunkY, chunkZ };
	}

	glm::ivec3 ChunkManager::BlockToChunkId(int x, int y, int z) const
	{
		int chunkX = x < 0 ? std::floor(x / (float)CHUNK_SIZE) : x / (float)CHUNK_SIZE;
		int chunkY = y < 0 ? std::floor(y / (float)CHUNK_SIZE) : y / (float)CHUNK_SIZE;
		int chunkZ = z < 0 ? std::floor(z / (float)CHUNK_SIZE) : z / (float)CHUNK_SIZE;

		return { chunkX, chunkY, chunkZ };
	}

	ChunkRef<ChunkData> ChunkManager::GetChunkDataFromId(const glm::ivec3& id)
	{
		std::shared_lock<std::shared_mutex> chunkDataLock(_chunkDataMutex);
		if (_chunkData.find(id) != _chunkData.end())
			return ChunkRef(&_chunkData[id]);

		return nullptr;
	}

	ChunkRef<ChunkData> ChunkManager::GetChunkDataFromWorldPos(const glm::vec3& pos)
	{
		auto chunkId = WorldToChunkId(pos);
		return GetChunkDataFromId(chunkId);
	}

	ChunkRef<ChunkData> ChunkManager::GetChunkDataFromWorldPos(float x, float y, float z)
	{
		auto chunkId = WorldToChunkId(x, y, z);
		return GetChunkDataFromId(chunkId);
	}

	void ChunkManager::Update()
	{
		_currentCamChunk = WorldToChunkId(_camera->position);
		if (_currentCamChunk != _lastCamChunk)
		{
			// Camera changed chunk
			//LOG_INFO("Camera changed chunks");
			_lastCamChunk = _currentCamChunk;

			{
				std::lock_guard lk(_chunkThreadMutex);
				_chunkQueueShouldReset = true;
			}
			_chunkThreadCv.notify_one();
		}
	}

	void ChunkManager::Render()
	{
		{
			std::shared_lock<std::shared_mutex> chunkRendererLock(_chunkRenderersMutex);
			for (auto& [id, renderer] : _chunkRenderers)
				renderer->Render();
		}
	}

	void ChunkManager::ResetQueue()
	{
		{
			std::lock_guard lk(_chunkThreadMutex);
			_chunkQueueShouldReset = true;
		}
		_chunkThreadCv.notify_one();
	}

	ChunkRef<ChunkData> ChunkManager::GetOrCreateChunkData(const glm::ivec3& id)
	{
		// Return it if it exists
		{
			std::shared_lock<std::shared_mutex> chunkDataLock(_chunkDataMutex);
			if (_chunkData.find(id) != _chunkData.end())
				return ChunkRef(&_chunkData[id]);
		}

		//LOG_INFO("Creating chunk data: %d %d %d", id.x, id.y, id.z);
		
		// It does not exist. Create a new one
		ChunkData* data = new ChunkData();
		_worldGen.GenerateChunk(id, data);

		// Add to unordered map and return reference
		std::unique_lock<std::shared_mutex> chunkDataModLock(_chunkDataMutex);
		auto [it, inserted] = _chunkData.emplace(id, data);

		//LOG_INFO("Chunk Data: %d", _chunkData.size());

		return ChunkRef<ChunkData>(it->second);
	}

	void ChunkManager::ChunkThreadUpdate()
	{
		while (!_chunkThreadShouldEnd)
		{
			//LOG_INFO("Chunk queue count: %d", _chunkQueue.size());

			// Delete chunk renderer out of render distance
			if (!_chunkRendererDeletionQueue.empty())
			{
				auto id = _chunkRendererDeletionQueue.front();
				_chunkRendererDeletionQueue.pop();
				{
					std::unique_lock<std::shared_mutex> chunkRendererLock(_chunkRenderersMutex);
					auto it = _chunkRenderers.find(id);
					if (it != _chunkRenderers.end() && it->second._refCount.load() == 0)
					{
						_chunkRenderers.erase(it);
						//LOG_INFO("Deleted chunk renderer %d %d %d. Chunk Renderers: %d", id.x, id.y, id.z, _chunkRenderers.size());
					}
				}
			}

			// Delete unused chunk data
			{
				std::unique_lock<std::shared_mutex> chunkDataLock(_chunkDataMutex);
				for (auto it = _chunkData.begin(); it != _chunkData.end();)
				{
					if (it->second._refCount.load() == 0)
					{
						//LOG_INFO("Deleting chunk data %d %d %d", it->first.x, it->first.y, it->first.z);
						it = _chunkData.erase(it);
						//LOG_INFO("Chunk Data: %d", _chunkData.size());
					}
					else
						++it;
				}
			}

			// Reset queue if camera moved to a different chunk
			// or if the chunk queue was manually reset
			bool queueShouldReset;
			{
				std::lock_guard lk(_chunkThreadMutex);
				queueShouldReset = _chunkQueueShouldReset;
			}
			if (queueShouldReset)
			{
				//LOG_INFO("Resetting queue");
				std::queue<glm::ivec3> newQueue;
				std::swap(_chunkQueue, newQueue);

				int chunkX = _currentCamChunk.x;
				int chunkY = _currentCamChunk.y;
				int chunkZ = _currentCamChunk.z;

				_chunkQueue.push({ chunkX, chunkY, chunkZ });

				for (int r = 0; r <= _renderDistance; r++)
				{
					// Add middle chunks
					for (int y = 0; y <= _renderHeight; y++)
					{
						_chunkQueue.push({ chunkX,     chunkY + y, chunkZ + r });
						_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ });
						_chunkQueue.push({ chunkX,     chunkY + y, chunkZ - r });
						_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ });

						if (y > 0)
						{
							_chunkQueue.push({ chunkX,     chunkY - y, chunkZ + r });
							_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ });
							_chunkQueue.push({ chunkX,     chunkY - y, chunkZ - r });
							_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ });
						}
					}

					// Add edges
					for (int e = 1; e < r; e++)
					{
						for (int y = 0; y <= _renderHeight; y++)
						{
							_chunkQueue.push({ chunkX + e, chunkY + y, chunkZ + r });
							_chunkQueue.push({ chunkX - e, chunkY + y, chunkZ + r });

							_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ + e });
							_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ - e });

							_chunkQueue.push({ chunkX + e, chunkY + y, chunkZ - r });
							_chunkQueue.push({ chunkX - e, chunkY + y, chunkZ - r });

							_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ + e });
							_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ - e });

							if (y > 0)
							{
								_chunkQueue.push({ chunkX + e, chunkY - y, chunkZ + r });
								_chunkQueue.push({ chunkX - e, chunkY - y, chunkZ + r });

								_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ + e });
								_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ - e });

								_chunkQueue.push({ chunkX + e, chunkY - y, chunkZ - r });
								_chunkQueue.push({ chunkX - e, chunkY - y, chunkZ - r });

								_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ + e });
								_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ - e });
							}
						}
					}

					// Add corners
					for (int y = 0; y <= _renderHeight; y++)
					{
						_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ + r });
						_chunkQueue.push({ chunkX + r, chunkY + y, chunkZ - r });
						_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ + r });
						_chunkQueue.push({ chunkX - r, chunkY + y, chunkZ - r });

						if (y > 0)
						{
							_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ + r });
							_chunkQueue.push({ chunkX + r, chunkY - y, chunkZ - r });
							_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ + r });
							_chunkQueue.push({ chunkX - r, chunkY - y, chunkZ - r });
						}
					}
				}

				{
					std::lock_guard lk(_chunkThreadMutex);
					_chunkQueueShouldReset = false;
				}

				// Add chunk renderers out of render distance to deletion queue
				while (!_chunkRendererDeletionQueue.empty())
					_chunkRendererDeletionQueue.pop();
				std::shared_lock<std::shared_mutex> chunkRendererLock(_chunkRenderersMutex);
				for (auto& [id, renderer] : _chunkRenderers)
				{
					// Check if out of bounds of render distance
					if (std::abs(id.x - _currentCamChunk.x) > _renderDistance ||
						std::abs(id.z - _currentCamChunk.z) > _renderDistance ||
						std::abs(id.y - _currentCamChunk.y) > _renderHeight)
					{
						_chunkRendererDeletionQueue.push(id);
					}
				}
			}

			// Create chunk renderer
			if (_chunkQueue.size() > 0)
			{
				// Grab first chunk renderer that doesn't exist
				glm::ivec3 chunkId;
				bool newChunk = false;
				while (_chunkQueue.size() > 0)
				{
					chunkId = _chunkQueue.front();
					_chunkQueue.pop();

					std::shared_lock<std::shared_mutex> chunkRendererReadLock(_chunkRenderersMutex);
					if (_chunkRenderers.find(chunkId) == _chunkRenderers.end())
					{
						newChunk = true;
						break;
					}
				}

				if (!newChunk) continue;

				//LOG_INFO("Creating chunk renderer for %d %d %d", chunkId.x, chunkId.y, chunkId.z);

				auto data = GetOrCreateChunkData(chunkId);
				auto northData = GetOrCreateChunkData({ chunkId.x, chunkId.y, chunkId.z - 1 });
				auto southData = GetOrCreateChunkData({ chunkId.x, chunkId.y, chunkId.z + 1 });
				auto eastData = GetOrCreateChunkData({ chunkId.x - 1, chunkId.y, chunkId.z });
				auto westData = GetOrCreateChunkData({ chunkId.x + 1, chunkId.y, chunkId.z });
				auto upData = GetOrCreateChunkData({ chunkId.x, chunkId.y + 1, chunkId.z });
				auto downData = GetOrCreateChunkData({ chunkId.x, chunkId.y - 1, chunkId.z });

				ChunkRenderer* renderer = new ChunkRenderer(chunkId, data, _shader, northData, southData,
					eastData, westData, upData, downData);
				renderer->CreateMesh();

				std::unique_lock<std::shared_mutex> chunkRendererLock(_chunkRenderersMutex);
				_chunkRenderers.emplace(chunkId, renderer);

				//LOG_INFO("Chunk Renderers: %d", _chunkRenderers.size());
			}
			else if (_chunkRendererDeletionQueue.empty())
			{
				// Chunk queue empty. Wait until camera moves
				//LOG_INFO("Chunk queue empty. Pausing...");
				std::unique_lock lk(_chunkThreadMutex);
				_chunkThreadCv.wait(lk, [this] {
					return _chunkQueueShouldReset || _chunkThreadShouldEnd;
				});
			}
		}
	}

	int ChunkManager::GetChunkRenderersSize()
	{
		std::shared_lock<std::shared_mutex> lock(_chunkRenderersMutex);
		return _chunkRenderers.size();
	}
}