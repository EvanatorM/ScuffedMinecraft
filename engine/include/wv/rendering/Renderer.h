#pragma once

#include <wv/rendering/Window.h>
#include <wv/rendering/Vertex.h>
#include <wv/rendering/Shader.h>
#include <wv/rendering/Mesh.h>
#include <wv/pch.h>

namespace WillowVox
{
    class Camera;

    class Renderer
    {
    public:
        Renderer(Window* window);
        ~Renderer();

        static Renderer* GetInstance()
        {
            return s_instance;
        }

        void BeginRendering();
        void EndRendering();

        void RenderMesh(const Shader& shader, const Mesh& mesh);

        Shader* CreateShader(const std::string& shaderPath);
        Mesh* CreateMesh(const Vertex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount);

        void DestroyMesh(Mesh* mesh);

        void UpdateUniformBuffer(const Camera& camera);
        void UpdatePushConstants(const Shader& shader, const glm::mat4& model);

        void SetClearColor(const glm::vec4& color);

        void Stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> _impl;

        static Renderer* s_instance;
    };
}