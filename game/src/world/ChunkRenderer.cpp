#include <world/ChunkRenderer.h>

#include <world/BlockRegistry.h>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	ChunkRenderer::ChunkRenderer(const glm::ivec3& id, ChunkRefOwner<ChunkData>& data, Shader* shader,
		ChunkRefOwner<ChunkData>& northData, ChunkRefOwner<ChunkData>& southData,
		ChunkRefOwner<ChunkData>& eastData, ChunkRefOwner<ChunkData>& westData,
		ChunkRefOwner<ChunkData>& upData, ChunkRefOwner<ChunkData>& downData)
		: _id(id), _data(ChunkRef<ChunkData>(data)), _shader(shader), _mesh(nullptr), _renderer(*Renderer::GetInstance()),
		_northData(ChunkRef<ChunkData>(northData)), _southData(ChunkRef<ChunkData>(southData)),
		_eastData(ChunkRef<ChunkData>(eastData)), _westData(ChunkRef<ChunkData>(westData)),
		_upData(ChunkRef<ChunkData>(upData)), _downData(ChunkRef<ChunkData>(downData))
	{
		_position = id * CHUNK_SIZE;
		_model = glm::translate(glm::mat4(1.0f), _position);
	}

	ChunkRenderer::ChunkRenderer(const glm::ivec3& id, const ChunkRef<ChunkData>& data, Shader* shader,
		const ChunkRef<ChunkData>& northData, const ChunkRef<ChunkData>& southData,
		const ChunkRef<ChunkData>& eastData, const ChunkRef<ChunkData>& westData,
		const ChunkRef<ChunkData>& upData, const ChunkRef<ChunkData>& downData)
		: _id(id), _data(data), _shader(shader), _mesh(nullptr), _renderer(*Renderer::GetInstance()),
		_northData(northData), _southData(southData),
		_eastData(eastData), _westData(westData),
		_upData(upData), _downData(downData)
	{
		_position = id * CHUNK_SIZE;
		_model = glm::translate(glm::mat4(1.0f), _position);
	}

	ChunkRenderer::~ChunkRenderer()
	{

	}

	static inline BlockId GetNorthBlock(ChunkData* data, ChunkData* northData, int x, int y, int z)
	{
		if (z == 0)
		{
			return northData->GetBlock(x, y, CHUNK_SIZE - 1);
		}
		else
			return data->GetBlock(x, y, z - 1);
	}

	static inline BlockId GetSouthBlock(ChunkData* data, ChunkData* southData, int x, int y, int z)
	{
		if (z == CHUNK_SIZE - 1)
		{
			return southData->GetBlock(x, y, 0);
		}
		else
			return data->GetBlock(x, y, z + 1);
	}

	static inline BlockId GetEastBlock(ChunkData* data, ChunkData* eastData, int x, int y, int z)
	{
		if (x == 0)
		{
			return eastData->GetBlock(CHUNK_SIZE - 1, y, z);
		}
		else
			return data->GetBlock(x - 1, y, z);
	}

	static inline BlockId GetWestBlock(ChunkData* data, ChunkData* westData, int x, int y, int z)
	{
		if (x == CHUNK_SIZE - 1)
		{
			return westData->GetBlock(0, y, z);
		}
		else
			return data->GetBlock(x + 1, y, z);
	}

	static inline BlockId GetUpBlock(ChunkData* data, ChunkData* upData, int x, int y, int z)
	{
		if (y == CHUNK_SIZE - 1)
		{
			return upData->GetBlock(x, 0, z);
		}
		else
			return data->GetBlock(x, y + 1, z);
	}

	static inline BlockId GetDownBlock(ChunkData* data, ChunkData* downData, int x, int y, int z)
	{
		if (y == 0)
		{
			return downData->GetBlock(x, CHUNK_SIZE - 1, z);
		}
		else
			return data->GetBlock(x, y - 1, z);
	}

	void ChunkRenderer::CreateMesh()
	{
		auto* blockRegistry = BlockRegistry::GetInstance();

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		uint32_t vertexCount = 0;
		int chunkI = -1;
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				for (int y = 0; y < CHUNK_SIZE; y++)
				{
					chunkI++;

					BlockId block = _data->GetBlock(chunkI);
					if (block <= AIR) // Skip if air (or invalid)
						continue;

					auto* blockDisplay = blockRegistry->GetBlockDisplay(block);

					// Create cube
					// North Face
					if (GetNorthBlock(_data.Get(), _northData.Get(), x, y, z) == AIR)
					{
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMaxY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMaxY));

						indices.push_back(vertexCount + 0);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 3);
						vertexCount += 4;
					}

					// South Face
					if (GetSouthBlock(_data.Get(), _southData.Get(), x, y, z) == AIR)
					{
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMaxY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMaxY));

						indices.push_back(vertexCount + 0);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 3);
						vertexCount += 4;
					}

					// East Face
					if (GetEastBlock(_data.Get(), _eastData.Get(), x, y, z) == AIR)
					{
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMaxY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMaxY));

						indices.push_back(vertexCount + 0);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 3);
						vertexCount += 4;
					}

					// West Face
					if (GetWestBlock(_data.Get(), _westData.Get(), x, y, z) == AIR)
					{
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMinY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f), glm::vec2(blockDisplay->sideTextureMinX, blockDisplay->sideTextureMaxY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f), glm::vec2(blockDisplay->sideTextureMaxX, blockDisplay->sideTextureMaxY));

						indices.push_back(vertexCount + 0);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 3);
						vertexCount += 4;
					}

					// Top Face
					if (GetUpBlock(_data.Get(), _upData.Get(), x, y, z) == AIR)
					{
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f), glm::vec2(blockDisplay->topTextureMinX, blockDisplay->topTextureMinY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f), glm::vec2(blockDisplay->topTextureMaxX, blockDisplay->topTextureMinY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f), glm::vec2(blockDisplay->topTextureMinX, blockDisplay->topTextureMaxY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f), glm::vec2(blockDisplay->topTextureMaxX, blockDisplay->topTextureMaxY));

						indices.push_back(vertexCount + 0);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 3);
						vertexCount += 4;
					}

					// Bottom Face
					if (GetDownBlock(_data.Get(), _downData.Get(), x, y, z) == AIR)
					{
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f), glm::vec2(blockDisplay->bottomTextureMinX, blockDisplay->bottomTextureMinY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f), glm::vec2(blockDisplay->bottomTextureMaxX, blockDisplay->bottomTextureMinY));
						vertices.emplace_back(glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f), glm::vec2(blockDisplay->bottomTextureMinX, blockDisplay->bottomTextureMaxY));
						vertices.emplace_back(glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f), glm::vec2(blockDisplay->bottomTextureMaxX, blockDisplay->bottomTextureMaxY));

						indices.push_back(vertexCount + 0);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 1);
						indices.push_back(vertexCount + 2);
						indices.push_back(vertexCount + 3);
						vertexCount += 4;
					}
				}
			}
		}

		//LOG_INFO("Vertices: %d, Indices: %d", vertices.size(), indices.size());
		_mesh.reset();
		_mesh = std::unique_ptr<Mesh>(_renderer.CreateMesh(vertices.data(), vertices.size(), indices.data(), indices.size()));
	}

	void ChunkRenderer::Render()
	{
		if (!_mesh)
			return;

		_renderer.UpdatePushConstants(*_shader, _model);
		_renderer.RenderMesh(*_shader, *_mesh);
	}
}