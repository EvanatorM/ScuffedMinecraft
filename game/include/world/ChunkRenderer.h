#pragma once

#include <world/ChunkData.h>
#include <world/ChunkRef.h>
#include <wv/core.h>

namespace ScuffedMinecraft
{
	class ChunkRenderer
	{
	public:
		ChunkRenderer(const glm::ivec3& id, ChunkRefOwner<ChunkData>& data, WillowVox::Shader* shader,
			ChunkRefOwner<ChunkData>& northData, ChunkRefOwner<ChunkData>& southData,
			ChunkRefOwner<ChunkData>& eastData, ChunkRefOwner<ChunkData>& westData,
			ChunkRefOwner<ChunkData>& upData, ChunkRefOwner<ChunkData>& downData);
		ChunkRenderer(const glm::ivec3& id, const ChunkRef<ChunkData>& data, WillowVox::Shader* shader,
			const ChunkRef<ChunkData>& northData, const ChunkRef<ChunkData>& southData,
			const ChunkRef<ChunkData>& eastData, const ChunkRef<ChunkData>& westData,
			const ChunkRef<ChunkData>& upData, const ChunkRef<ChunkData>& downData);
		~ChunkRenderer();

		void CreateMesh();
		void Render();

	private:
		ChunkRef<ChunkData> _data;
		ChunkRef<ChunkData> _northData;
		ChunkRef<ChunkData> _southData;
		ChunkRef<ChunkData> _eastData;
		ChunkRef<ChunkData> _westData;
		ChunkRef<ChunkData> _upData;
		ChunkRef<ChunkData> _downData;

		WillowVox::Shader* _shader;
		std::unique_ptr<WillowVox::Mesh> _mesh;
		WillowVox::Renderer& _renderer;

		glm::ivec3 _id;
		glm::vec3 _position;
		glm::mat4 _model;
	};
}