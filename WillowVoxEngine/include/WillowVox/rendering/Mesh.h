#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <WillowVox/rendering/BaseVertex.h>
#include <cstdint>
#include <glm/glm.hpp>

namespace WillowVox
{
	enum PolygonMode
	{
		Triangle,
		Line
	};

	class WILLOWVOX_API Mesh
	{
	public:
		virtual void Render(const glm::vec3& position, BaseMaterial& material, const PolygonMode& mode = PolygonMode::Triangle) = 0; // Bind material and render
		virtual void Render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& position, BaseMaterial& material, const PolygonMode& mode = PolygonMode::Triangle) = 0; // Bind material and render
		virtual void RenderAsInstance(const glm::vec3& position, BaseMaterial& material, const PolygonMode& mode = PolygonMode::Triangle) = 0; // Render without binding material

		virtual void SetMesh(BaseVertex* vertices, uint32_t vertexTypeSize, int numVertices, uint32_t* indices, int numIndices) = 0;
		virtual void SetVertexProperties(BaseMaterial& material) = 0;
	};
}