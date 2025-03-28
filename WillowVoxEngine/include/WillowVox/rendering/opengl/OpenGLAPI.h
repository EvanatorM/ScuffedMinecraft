#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/RenderingAPI.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace WillowVox
{
	class WILLOWVOX_API OpenGLAPI : public RenderingAPI
	{
	public:
		OpenGLAPI();
		~OpenGLAPI();

		// Rendering objects
		Window* CreateWindow(int width, int height, const char* title) override;
		Shader* CreateShader(const char* vertexShaderPath, const char* fragmentShaderPath) override;
		Shader* CreateShaderFromString(const char* vertexShaderCode, const char* fragmentShaderCode) override;
		Mesh* CreateMesh() override;
		Texture* CreateTexture(const char* path) override;

		// Vertex attributes
		void SetVertexAttrib1f(int id, uint32_t size, std::size_t offset) override;
		void SetVertexAttrib2f(int id, uint32_t size, std::size_t offset) override;
		void SetVertexAttrib3f(int id, uint32_t size, std::size_t offset) override;
		void SetVertexAttrib1b(int id, uint32_t size, std::size_t offset) override;
		void SetVertexAttrib2b(int id, uint32_t size, std::size_t offset) override;
		void SetVertexAttrib3b(int id, uint32_t size, std::size_t offset) override;

		// Getters
		double GetTime() override;

		// Setters
		void SetCullFace(bool enabled) override;
		void SetDepthTest(bool enabled) override;
		void SetBlending(bool enabled) override;
		void SetInvertRenderMode(bool enabled) override;
		void SetRenderingMode(RenderMode mode) override;
		void SetLineWidth(float width) override;
		void SetVsync(bool enabled) override;

		// Raw (mostly debug) rendering
		void RenderTriangles(glm::vec3* vertices, int vertexCount, glm::vec4 color) override;

		static constexpr int OPENGL_VERSION_MAJOR = 3;
		static constexpr int OPENGL_VERSION_MINOR = 3;

	private:
		Shader* _debugShader;
	};
}