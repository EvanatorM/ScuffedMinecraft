#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/Window.h>
#include <WillowVox/rendering/Shader.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <WillowVox/rendering/Mesh.h>
#include <WillowVox/rendering/Texture.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <cstddef>

namespace WillowVox
{
	enum RenderMode
	{
		Fill,
		Wireframe
	};

	class WILLOWVOX_API RenderingAPI
	{
	public:
		// Function to get the correct rendering API without having to include unused APIs
		static RenderingAPI* GetRenderingAPI();

		// Rendering objects
		virtual Window* CreateWindow(int width, int height, const char* title) = 0;
		virtual Shader* CreateShader(const char* vertexShaderPath, const char* fragmentShaderPath) = 0;
		virtual Shader* CreateShaderFromString(const char* vertexShaderCode, const char* fragmentShaderCode) = 0;
		virtual Mesh* CreateMesh() = 0;
		virtual Texture* CreateTexture(const char* path) = 0;

		// Vertex attributes
		virtual void SetVertexAttrib1f(int id, uint32_t size, std::size_t offset) = 0;
		virtual void SetVertexAttrib2f(int id, uint32_t size, std::size_t offset) = 0;
		virtual void SetVertexAttrib3f(int id, uint32_t size, std::size_t offset) = 0;
		virtual void SetVertexAttrib1b(int id, uint32_t size, std::size_t offset) = 0;
		virtual void SetVertexAttrib2b(int id, uint32_t size, std::size_t offset) = 0;
		virtual void SetVertexAttrib3b(int id, uint32_t size, std::size_t offset) = 0;
		
		// Getters
		virtual double GetTime() = 0;

		// Setters
		virtual void SetCullFace(bool enabled) = 0;
		virtual void SetDepthTest(bool enabled) = 0;
		virtual void SetBlending(bool enabled) = 0;
		virtual void SetInvertRenderMode(bool enabled) = 0;
		virtual void SetRenderingMode(RenderMode mode) = 0;
		virtual void SetLineWidth(float width) = 0;
		virtual void SetVsync(bool enabled) = 0;

		// Raw (mostly debug) rendering
		virtual void RenderTriangles(glm::vec3* vertices, int vertexCount, glm::vec4 color) = 0;

		static RenderingAPI* m_renderingAPI;
	};
}