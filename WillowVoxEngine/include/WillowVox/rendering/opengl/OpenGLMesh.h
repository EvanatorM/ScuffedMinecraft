#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/Mesh.h>

namespace WillowVox
{
	class WILLOWVOX_API OpenGLMesh : public Mesh
	{
	public:
		OpenGLMesh();
		~OpenGLMesh();

		void Render(const glm::vec3& position, BaseMaterial& material, const PolygonMode& mode = PolygonMode::Triangle) override; // Bind material and render
		void Render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& position, BaseMaterial& material, const PolygonMode& mode = PolygonMode::Triangle) override;
		void RenderAsInstance(const glm::vec3& position, BaseMaterial& material, const PolygonMode& mode = PolygonMode::Triangle) override; // Render without binding material

		void SetMesh(BaseVertex* vertices, uint32_t vertexTypeSize, int numVertices, uint32_t* indices, int numIndices) override;
		void SetVertexProperties(BaseMaterial& material) override;
	
	private:
		unsigned int _vao;
		unsigned int _vbo;
		unsigned int _ebo;
		int _numTriangles;
	};
}