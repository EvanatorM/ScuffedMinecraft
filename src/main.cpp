#include <wv/core.h>
#include <wv/app/EntryPoint.h>
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <wv/rendering/VertexArrayObject.h>
#include <wv/VoxelWorlds.h>
#include <world/ScuffedWorldGen.h>
#include <ui/UIManager.h>
#include <cstdlib>

using namespace WillowVox;

const char* WillowVox::appWindowName = "Scuffed Minecraft";
int WillowVox::appDefaultWindowX = 1920;
int WillowVox::appDefaultWindowY = 1080;

namespace ScuffedMinecraft
{
    class ScuffedMinecraftApp : public App
    {
        void Start() override
        {
            // Init ImGUI
            UIManager::InitImGUI();

            // Set window background color
            Window::GetInstance().SetBackgroundColor(0.6f, 0.7f, 0.95f, 1.0f);

            // Load shader
            auto& am = AssetManager::GetInstance();
            m_chunkShader = am.GetAsset<Shader>("chunk_shader");

            m_camera = std::make_unique<Camera>();

            // Register blocks
            auto& blockRegistry = BlockRegistry::GetInstance();
            blockRegistry.RegisterBlock("dirt_block", "dirt_block.png");
            blockRegistry.RegisterBlock("grass_block", "grass_block.png", "dirt_block.png", "grass_block_side.png");
            blockRegistry.RegisterBlock("stone_block", "stone_block.png");

            blockRegistry.ApplyRegistry();

            ScuffedWorldGen::InitWorldGen();

            // Create chunk manager
            worldGen = std::make_unique<ScuffedWorldGen>();
            chunkManager = std::make_unique<ChunkManager>(worldGen.get(), 0, 0, 10, 0);
            chunkManager->SetCamera(m_camera.get());
            chunkManager->SetRenderDistance(5, 2);

            Input::SetMouseMode(MouseMode::DISABLED);

            m_registry = &BlockRegistry::GetInstance();
        }

        void Update() override
        {
            if (Input::GetKeyDown(Key::ESC))
            {
                m_mouseDisabled = !m_mouseDisabled;
                if (m_mouseDisabled)
                    Input::SetMouseMode(MouseMode::DISABLED);
                else
                    Input::SetMouseMode(MouseMode::NORMAL);
            }

            if (m_mouseDisabled)
            {
                if (Input::GetKey(Key::W))
                {
                    if (Input::GetKey(Key::SPACE))
                    {
                        auto front = m_camera->Front();
                        front.y = 0;
                        front = glm::normalize(front);
                        m_camera->m_position += front * m_speed * m_deltaTime;
                    }
                    else
                        m_camera->m_position += m_camera->Front() * m_speed * m_deltaTime;
                }
                if (Input::GetKey(Key::S))
                {
                    if (Input::GetKey(Key::SPACE))
                    {
                        auto front = m_camera->Front();
                        front.y = 0;
                        front = glm::normalize(front);
                        m_camera->m_position -= front * m_speed * m_deltaTime;
                    }
                    else
                        m_camera->m_position -= m_camera->Front() * m_speed * m_deltaTime;
                }
                if (Input::GetKey(Key::A))
                    m_camera->m_position -= m_camera->Right() * m_speed * m_deltaTime;
                if (Input::GetKey(Key::D))
                    m_camera->m_position += m_camera->Right() * m_speed * m_deltaTime;
                if (Input::GetKey(Key::E))
                    m_camera->m_position += m_camera->Up() * m_speed * m_deltaTime;
                if (Input::GetKey(Key::Q))
                    m_camera->m_position -= m_camera->Up() * m_speed * m_deltaTime;

                m_speed += Input::GetMouseScrollDelta().y;
                if (m_speed < 0.0f)
                    m_speed = 0;
                else if (m_speed > 20.0f)
                    m_speed = 20;

                auto mouseMove = Input::GetMouseDelta();
                m_camera->m_direction.y += mouseMove.x * m_sensitivity;
                m_camera->m_direction.x -= mouseMove.y * m_sensitivity;

                if (m_camera->m_direction.x > 89.0f)
                    m_camera->m_direction.x = 89.0f;
                if (m_camera->m_direction.x < -89.0f)
                    m_camera->m_direction.x = -89.0f;

                if (Input::GetMouseButtonDown(0))
                {
                    auto result = VoxelRaycast(*chunkManager, m_camera->m_position, m_camera->Front(), m_maxReach);
                    
                    if (result.hit)
                        chunkManager->SetBlockId(result.hitX, result.hitY, result.hitZ, 0);
                }
                if (Input::GetMouseButtonDown(1))
                {
                    auto result = VoxelRaycast(*chunkManager, m_camera->m_position, m_camera->Front(), m_maxReach);

                    if (result.hit)
                    {
                        auto block = chunkManager->WorldToBlockPos(result.hitX, result.hitY, result.hitZ);
                        float distX = result.hitX - (block.x + .5f);
                        float distY = result.hitY - (block.y + .5f);
                        float distZ = result.hitZ - (block.z + .5f);

                        int blockX = block.x;
                        int blockY = block.y;
                        int blockZ = block.z;

                        // Choose face to place on
                        if (abs(distX) > abs(distY) && abs(distX) > abs(distZ))
                            blockX += (distX > 0 ? 1 : -1);
                        else if (abs(distY) > abs(distX) && abs(distY) > abs(distZ))
                            blockY += (distY > 0 ? 1 : -1);
                        else
                            blockZ += (distZ > 0 ? 1 : -1);

                        chunkManager->SetBlockId(blockX, blockY, blockZ, m_selectedBlock);
                    }
                }
                if (Input::GetMouseButtonDown(2))
                {
                    auto result = VoxelRaycast(*chunkManager, m_camera->m_position, m_camera->Front(), m_maxReach);

                    if (result.hit)
                        m_selectedBlock = chunkManager->GetBlockId(result.hitX, result.hitY, result.hitZ);
                }
            }
        }

        void Render() override
        {
            auto proj = m_camera->GetProjectionMatrix();
            auto view = m_camera->GetViewMatrix();


            m_chunkShader->Bind();
            m_chunkShader->SetMat4("proj", proj);
            m_chunkShader->SetMat4("view", view);

            chunkManager->Render();

            RenderUI();
        }

        void RenderUI()
        {
            UIManager::BeginFrame();

            double currentTime = glfwGetTime();
            m_frameCount++;
            // If a second has passed.
            if (currentTime - m_previousTime >= 1.0)
            {
                // Display the frame count here any way you want.
                m_fps = m_frameCount;

                m_frameCount = 0;
                m_previousTime = currentTime;
            }

            ImGui::Text("FPS: %d", m_fps);
            ImGui::Text("Camera Pos: %f %f %f (%d %d %d)", m_camera->m_position.x, m_camera->m_position.y, m_camera->m_position.z, 
                (int)(m_camera->m_position.x < 0 ? (m_camera->m_position.x / CHUNK_SIZE) - 1 : m_camera->m_position.x / CHUNK_SIZE), 
                (int)(m_camera->m_position.y < 0 ? (m_camera->m_position.y / CHUNK_SIZE) - 1 : m_camera->m_position.y / CHUNK_SIZE), 
                (int)(m_camera->m_position.z < 0 ? (m_camera->m_position.z / CHUNK_SIZE) - 1 : m_camera->m_position.z / CHUNK_SIZE));
            ImGui::Text("Selected Block: %s", m_registry->GetBlock(m_selectedBlock).strId.c_str());
            ImGui::Checkbox("Vysnc", &m_vsync);
            if (m_vsync != Renderer::VysncEnabled())
                Renderer::SetVsync(m_vsync);
            #ifdef DEBUG_MODE
            ImGui::Text("Avg time to generate chunk data: %f ms", chunkManager->m_avgChunkDataGenTime);
            ImGui::Text("Avg time to generate chunk mesh: %f ms", ChunkRenderer::m_avgMeshGenTime);
            #endif

            UIManager::EndFrame();
        }

        double m_previousTime = 0;
        int m_frameCount = 0;
        int m_fps = 0;

        std::unique_ptr<WorldGen> worldGen;
        std::unique_ptr<ChunkManager> chunkManager;

        std::unique_ptr<Camera> m_camera;

        std::shared_ptr<Shader> m_chunkShader;

        float m_speed = 5.0f;
        float m_sensitivity = 0.5f;
        bool m_mouseDisabled = true;

        float m_maxReach = 5.0f;
        BlockId m_selectedBlock = 1;

        BlockRegistry* m_registry;

        bool m_vsync = true;
    };
}

WillowVox::App* WillowVox::CreateApp()
{
    return new ScuffedMinecraft::ScuffedMinecraftApp();
}