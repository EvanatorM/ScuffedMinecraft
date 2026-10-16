#include <wv/core.h>

#include <world/ChunkData.h>
#include <world/ChunkRenderer.h>
#include <world/BlockDisplay.h>
#include <world/ChunkManager.h>
#include <world/ChunkRef.h>
#include <world/Block.h>
#include <world/BlockRegistry.h>
#include <FastNoise/FastNoise.h>
#include <cstdlib>

using namespace WillowVox;

namespace ScuffedMinecraft
{
    class ScuffedMinecraftApp : public WillowVox::App
    {
    protected:
        void Start() override
        {
            LOG_INFO("ScuffedMinecraftApp started!");

            _renderer->SetClearColor({ 0.4f, 0.6f, 0.9f, 1.0f });

            // Load assets
            _shader = std::unique_ptr<Shader>(_renderer->CreateShader("assets/shaders/shader.spv"));

            // Register blocks
            BlockRegistry* blockRegistry = BlockRegistry::GetInstance();
            BlockId dirtBlockId = blockRegistry->RegisterBlock({ "Dirt Block" }, { 0.0f, 0.5f, 0.0f, 0.5f });
            BlockId grassBlockId = blockRegistry->RegisterBlock({ "Grass Block" }, 
                { 0.5f, 1.0f, 0.0f, 0.5f,  0.0f, 0.5f, 0.0f, 0.5f,  0.0f, 0.5f, 0.5f, 1.0f });

            _camera = std::make_unique<Camera>();
            _camera->position = { 0.0f, 15.0f, 0.0f };

            Input::SetMouseMode(MouseMode::DISABLED);

            // Create chunk manager
            _chunkManager = std::make_unique<ChunkManager>(_shader.get(), _camera.get());
        }

        void Update(float deltaTime) override
        {
            _frames++;
            _frameTimer += deltaTime;
            if (_frameTimer >= 1.0f)
            {
                LOG_INFO("FPS: %d, Chunk Renderers: %d", _frames, _chunkManager->GetChunkRenderersSize());
                _frames = 0;
                _frameTimer -= 1.0f;
            }

            if (Input::GetKeyDown(Key::ESC))
            {
                _movementDisabled = !_movementDisabled;
                if (_movementDisabled)
                    Input::SetMouseMode(MouseMode::NORMAL);
                else
                    Input::SetMouseMode(MouseMode::DISABLED);
            }

            if (_movementDisabled)
                return;

            if (Input::GetKey(Key::SPACE))
            {
                if (Input::GetKey(Key::W))
                {
                    auto front = _camera->Front();
                    front.y = 0.0f;
                    front = glm::normalize(front);
                    _camera->position += front * _movementSpeed * deltaTime;
                }
                if (Input::GetKey(Key::S))
                {
                    auto front = _camera->Front();
                    front.y = 0.0f;
                    front = glm::normalize(front);
                    _camera->position -= front * _movementSpeed * deltaTime;
                }
            }
            else
            {
                if (Input::GetKey(Key::W))
                    _camera->position += _camera->Front() * _movementSpeed * deltaTime;
                if (Input::GetKey(Key::S))
                    _camera->position -= _camera->Front() * _movementSpeed * deltaTime;
            }
            if (Input::GetKey(Key::A))
                _camera->position -= _camera->Right() * _movementSpeed * deltaTime;
            if (Input::GetKey(Key::D))
                _camera->position += _camera->Right() * _movementSpeed * deltaTime;
            if (Input::GetKey(Key::E))
                _camera->position += _camera->Up() * _movementSpeed * deltaTime;
            if (Input::GetKey(Key::Q))
                _camera->position -= _camera->Up() * _movementSpeed * deltaTime;

            auto mouseMove = Input::GetMouseDelta();
            _camera->direction.y += mouseMove.x * _mouseSensitivity;
            _camera->direction.x -= mouseMove.y * _mouseSensitivity;
            if (_camera->direction.x >= 89.9f)
                _camera->direction.x = 89.9f;
            else if (_camera->direction.x <= -89.9f)
                _camera->direction.x = -89.9f;

            auto scrollDelta = Input::GetMouseScrollDelta();
            _movementSpeed += scrollDelta.y;
            if (_movementSpeed < 0.0f)
                _movementSpeed = 0.0f;
            else if (_movementSpeed > 100.0f)
                _movementSpeed = 100.0f;

            _chunkManager->Update();

            //LOG_INFO("Position: (%f, %f, %f)", _camera->position.x, _camera->position.y, _camera->position.z);
        }

        void Render() override
        {
            _chunkManager->Render();

            _renderer->UpdateUniformBuffer(*_camera);
        }

    private:
        std::unique_ptr<Shader> _shader;
        std::unique_ptr<Camera> _camera;
        std::unique_ptr<ChunkManager> _chunkManager;

        bool _movementDisabled = false;
        float _movementSpeed = 10.0f;
        float _mouseSensitivity = 0.3f;

        int _frames = 0;
        float _frameTimer = 0;
    };
}

#include <wv/app/EntryPoint.h>

WillowVox::App* WillowVox::CreateApp()
{
    auto app = new ScuffedMinecraft::ScuffedMinecraftApp();

    app->config.name = "ScuffedMinecraft";
    app->config.width = 1920;
    app->config.height = 1080;

    return app;
}