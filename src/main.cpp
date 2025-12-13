#include <wv/core.h>
#include <wv/app/EntryPoint.h>
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <wv/rendering/VertexArrayObject.h>
#include <wv/VoxelWorlds.h>
#include <world/ScuffedWorldGen.h>
#include <ui/UIManager.h>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>
#undef min
#undef max

using namespace WillowVox;

const char* WillowVox::appWindowName = "Scuffed Minecraft";
int WillowVox::appDefaultWindowX = 1920;
int WillowVox::appDefaultWindowY = 1080;

namespace ScuffedMinecraft
{
    std::vector<std::string> splitString(const std::string& str, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

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
            m_registry = &BlockRegistry::GetInstance();
            m_registry->RegisterBlock("dirt_block", "dirt_block.png");
            m_registry->RegisterBlock("grass_block", "grass_block.png", "dirt_block.png", "grass_block_side.png");
            m_registry->RegisterBlock("stone_block", "stone_block.png");
            m_registry->RegisterBlock("glowstone", "glowstone.png", true);

            m_registry->ApplyRegistry();

            m_selectedBlock = m_registry->GetBlockId("glowstone");

            ScuffedWorldGen::InitWorldGen();

            // Create chunk manager
            worldGen = std::make_unique<ScuffedWorldGen>();
            int numThreads = std::thread::hardware_concurrency();
            Logger::Log("Num available threads: %d", numThreads);
            chunkManager = std::make_unique<ChunkManager>(worldGen.get(), std::max((numThreads - 2) / 2, 2), 0, 0, 10, 0);
            chunkManager->SetCamera(m_camera.get());
            chunkManager->SetRenderDistance(5, 2);

            Input::SetMouseMode(MouseMode::DISABLED);

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

            if (Input::GetKeyDown(Key::F1))
            {
                m_uiEnabled = !m_uiEnabled;
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
                        if (std::fabs(distX) > std::fabs(distY) && std::fabs(distX) > std::fabs(distZ))
                            blockX += (distX > 0 ? 1 : -1);
                        else if (std::fabs(distY) > std::fabs(distX) && std::fabs(distY) > std::fabs(distZ))
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

        void ProcessCommand(const std::string& cmd)
        {
            try {
                if (cmd.size() == 0) return;

                auto tokens = splitString(cmd, ' ');
                if (tokens[0] == "tp")
                {
                    if (tokens.size() != 4 && tokens.size() != 7)
                    {
                        Logger::Error("Invalid number of arguments for command 'tp'. Expected format: 'tp xPos yPos zPos [xDir yDir zDir]");
                        return;
                    }
                    float x, y, z;
                    x = std::stof(tokens[1]);
                    y = std::stof(tokens[2]);
                    z = std::stof(tokens[3]);

                    m_camera->m_position = { x, y, z };

                    if (tokens.size() == 7)
                    {
                        float xDir, yDir, zDir;
                        xDir = std::stof(tokens[4]);
                        yDir = std::stof(tokens[5]);
                        zDir = std::stof(tokens[6]);

                        m_camera->m_direction = { xDir, yDir, zDir };
                    }
                }
                else if (tokens[0] == "select")
                {
                    if (tokens.size() != 2)
                    {
                        Logger::Error("Invalid number of arguments for command 'select'. Expected format: 'select blockId");
                        return;
                    }

                    int id;
                    bool intId = false;
                    try {
                        id = std::stoi(tokens[1]);
                        intId = true;
                    } catch(...) {}

                    if (intId)
                    {
                        try {
                            auto& b = m_registry->GetBlock(id);
                            m_selectedBlock = b.id;
                        }
                        catch (...) {}
                    }
                    else
                    {
                        try {
                            auto& b = m_registry->GetBlock(tokens[1]);
                            m_selectedBlock = b.id;
                        } catch (...) {}
                    }
                }
            }
            catch (const std::runtime_error& e) {
                Logger::Error("Error running command 'cmd': %s", e.what());
            }
            catch (...) {
                Logger::Error("Error running command 'cmd'");
            }
        }

        void RenderUI()
        {
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

            if (!m_uiEnabled)
                return;

            UIManager::BeginFrame();

            ImGui::Text("FPS: %d", m_fps);
            ImGui::Text("Camera Pos: %f %f %f (%d %d %d)", m_camera->m_position.x, m_camera->m_position.y, m_camera->m_position.z, 
                (int)(m_camera->m_position.x < 0 ? (m_camera->m_position.x / CHUNK_SIZE) - 1 : m_camera->m_position.x / CHUNK_SIZE), 
                (int)(m_camera->m_position.y < 0 ? (m_camera->m_position.y / CHUNK_SIZE) - 1 : m_camera->m_position.y / CHUNK_SIZE), 
                (int)(m_camera->m_position.z < 0 ? (m_camera->m_position.z / CHUNK_SIZE) - 1 : m_camera->m_position.z / CHUNK_SIZE));
            ImGui::Text("Camera Direction: %f %f %f (%d %d %d)", m_camera->m_direction.x, m_camera->m_direction.y, m_camera->m_direction.z);
            auto& b = m_registry->GetBlock(m_selectedBlock);
            ImGui::Text("Selected Block: %s (%d)", b.strId.c_str(), b.id);
            ImGui::Checkbox("Vysnc", &m_vsync);
            if (m_vsync != Renderer::VysncEnabled())
                Renderer::SetVsync(m_vsync);

            #ifdef DEBUG_MODE
            ImGui::Text("Avg time to generate chunk data: %f ms", chunkManager->m_avgChunkDataGenTime);
            ImGui::Text("Avg time to generate chunk mesh: %f ms", ChunkRenderer::m_avgMeshGenTime);
            #endif

            ImGui::InputText(" ", commandBuffer, 256);
            ImGui::SameLine();
            if (ImGui::Button("Run Command"))
            {
                ProcessCommand(std::string(commandBuffer));
            }

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
        BlockId m_selectedBlock = 0;

        BlockRegistry* m_registry;

        bool m_vsync = true;

        char commandBuffer[256] = "";

        bool m_uiEnabled = true;
    };
}

WillowVox::App* WillowVox::CreateApp()
{
    return new ScuffedMinecraft::ScuffedMinecraftApp();
}
