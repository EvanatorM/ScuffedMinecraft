#pragma once

#include <WillowVox/core/Application.h>
#include <WillowVox/rendering/Mesh.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <WillowVox/rendering/Camera.h>
#include <WillowVox/rendering/Texture.h>
#include <WillowVox/core/EntryPoint.h>
#include <WillowVox/core/Logger.h>
#include <WillowVox/rendering/engine-default/TextureMaterial.h>
#include <WillowVox/rendering/engine-default/Vertex.h>
#include <WillowVox/rendering/engine-default/ChunkVertex.h>
#include <WillowVox/rendering/engine-default/ChunkSolidMaterial.h>
#include <WillowVox/rendering/engine-default/TextureMaterial.h>
#include <WillowVox/physics/Physics.h>
#include <WillowVox/resources/Blocks.h>
#include <StandardWorld.h>
#include <BlockOutlineMaterial.h>
#include <BlockOutlineVertex.h>
#include <CrosshairMaterial.h>
#include <CrosshairVertex.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	class ScuffedMinecraft : public Application
	{
	public:
		ScuffedMinecraft()
		{
			_applicationName = "Scuffed Minecraft";
			_defaultWindowWidth = 1920;
			_defaultWindowHeight = 1080;
		}

		~ScuffedMinecraft()
		{
			delete _camera;
		}

		void LoadAssets() override
		{

		}

		void RegisterBlocks() override
		{
			Blocks::RegisterBlock({ 1, 1, 0, 0, 1, 0, Block::SOLID, "Grass Block" });
			Blocks::RegisterBlock({ 0, 0, Block::SOLID, "Dirt Block" });
			Blocks::RegisterBlock({ 0, 1, Block::SOLID, "Stone Block" });
			Blocks::RegisterBlock({ 0, 4, Block::LIQUID, "Water" });
			Blocks::RegisterBlock({ 2, 1, 2, 1, 2, 0, Block::SOLID, "Log" });
			Blocks::RegisterBlock({ 0, 2, Block::LEAVES, "Leaves" });
			Blocks::RegisterBlock({ 1, 2, Block::BILLBOARD, "Grass" });
			Blocks::RegisterBlock({ 3, 0, Block::BILLBOARD, "Tall Grass Bottom" });
			Blocks::RegisterBlock({ 3, 1, Block::BILLBOARD, "Tall Grass Top" });
			Blocks::RegisterBlock({ 0, 3, Block::BILLBOARD, "Poppy" });
			Blocks::RegisterBlock({ 1, 3, Block::BILLBOARD, "Orange Tulip" });
			Blocks::RegisterBlock({ 2, 2, Block::BILLBOARD, "White Tulip" });
			Blocks::RegisterBlock({ 3, 2, Block::BILLBOARD, "Pink Tulip" });
			Blocks::RegisterBlock({ 4, 0, Block::SOLID, "Sand" });
		}

		void Start() override
		{
			Logger::Log("Started Scuffed Minecraft v0.1.0");

			_window->SetBackgroundColor(0.6f, 0.8f, 1.0f, 1.0f);
			_renderingAPI->SetCullFace(true);
			_renderingAPI->SetDepthTest(true);
			_renderingAPI->SetLineWidth(2.0f);

			_camera = new WillowVox::Camera(_window);

			m_world = new StandardWorld(_camera);

			_window->SetMouseDisabled(true);

			_window->MouseMoveEventDispatcher.RegisterListener([this](MouseMoveEvent& e) {
				OnMouseMove(e.m_xOffset, e.m_yOffset);
			});

			_window->KeyPressEventDispatcher.RegisterListener([this](KeyPressEvent& e) {
				OnKeyPressed(e.m_key);
			});

			_window->MouseScrollEventDispatcher.RegisterListener([this](MouseScrollEvent& e) {
				if (this->_paused)
					return;

				this->_moveSpeed += (float)e.m_yOffset;
				if (this->_moveSpeed < 0)
					this->_moveSpeed = 0;
			});

			_window->MouseClickEventDispatcher.RegisterListener([this](MouseClickEvent& e) {
				OnMouseClick(e.m_button);
			});

			_window->WindowResizeEventDispatcher.RegisterListener([this](WindowResizeEvent& e) {
				CrosshairVertex crosshairVertices[] = {
				{ { (float)e.m_newWidth / 2 - 13.5, (float)e.m_newHeight / 2 - 13.5 }, { 0.0f, 0.0f }},
				{ { (float)e.m_newWidth / 2 + 13.5, (float)e.m_newHeight / 2 - 13.5 }, { 1.0f, 0.0f }},
				{ { (float)e.m_newWidth / 2 - 13.5, (float)e.m_newHeight / 2 + 13.5 }, { 0.0f, 1.0f }},
				{ { (float)e.m_newWidth / 2 + 13.5, (float)e.m_newHeight / 2 + 13.5 }, { 1.0f, 1.0f }},
				};

				uint32_t crosshairIndices[] = {
					0, 2, 1,
					1, 2, 3
				};

				Mesh* crosshairMesh = _renderingAPI->CreateMesh();
				crosshairMesh->SetMesh(crosshairVertices, sizeof(CrosshairVertex), 4, crosshairIndices, 6);
				_crosshairMesh->SetMesh(crosshairMesh, true);

				glm::mat4 ortho = glm::ortho(0.0f, (float)e.m_newWidth, (float)e.m_newHeight, 0.0f, 0.0f, 10.0f);
				this->_crosshairShader->Bind();
				this->_crosshairShader->SetMat4("projection", ortho);
			});

			_underwaterShader = new PostProcessingShader(_renderingAPI->CreateShader("assets/shaders/post-processing/test_vert.glsl", "assets/shaders/post-processing/test_frag.glsl"), false);
			_window->AddPostProcessingShader(_underwaterShader);

			_renderingAPI->SetVsync(_vsync);

			CreateUIStuff();
		}

		void Update() override
		{
			if (_paused)
				return;

			// Camera movement
			if (_window->KeyDown(Key::W))
			{
				if (_window->KeyDown(Key::SPACE))
				{
					glm::vec3 dir = _camera->Front();
					dir.y = 0;
					dir = glm::normalize(dir);
					_camera->position += dir * _moveSpeed * m_deltaTime;
				}
				else
					_camera->position += _camera->Front() * _moveSpeed * m_deltaTime;
			}
			if (_window->KeyDown(Key::S))
			{
				if (_window->KeyDown(Key::SPACE))
				{
					glm::vec3 dir = _camera->Front();
					dir.y = 0;
					dir = glm::normalize(dir);
					_camera->position += -dir * _moveSpeed * m_deltaTime;
				}
				else
					_camera->position += -_camera->Front() * _moveSpeed * m_deltaTime;
			}
			if (_window->KeyDown(Key::A))
				_camera->position -= _camera->Right() * _moveSpeed * m_deltaTime;
			if (_window->KeyDown(Key::D))
				_camera->position += _camera->Right() * _moveSpeed * m_deltaTime;
			if (_window->KeyDown(Key::E))
				_camera->position += _camera->Up() * _moveSpeed * m_deltaTime;
			if (_window->KeyDown(Key::Q))
				_camera->position -= _camera->Up() * _moveSpeed * m_deltaTime;

			_camera->direction.z += 10.0f * m_deltaTime;
		}

		void OnMouseMove(float x, float y)
		{
			if (_paused)
				return;

			if (_firstFrame)
			{
				_firstFrame = false;
				return;
			}

			_camera->direction.y += x * _mouseSensitivity;
			_camera->direction.x -= y * _mouseSensitivity;
			if (_camera->direction.x > 89.0f)
				_camera->direction.x = 89.0f;
			else if (_camera->direction.x < -89.0f)
				_camera->direction.x = -89.0f;
		}

		void OnKeyPressed(Key key)
		{
			if (key == Key::ESC)
			{
				_paused = !_paused;
				_window->SetMouseDisabled(!_paused);
				_firstFrame = true;
			}
			else if (key == Key::F1)
				_renderUI = !_renderUI;
		}

		void OnMouseClick(int button)
		{
			if (_paused)
				return;

			switch (button)
			{
			case 0: // Left click
			{
				auto result = Physics::Raycast(*m_world->m_chunkManager, _camera->position, _camera->Front(), 10.0f);
				if (result.m_hit)
					result.m_chunk->SetBlock(result.m_localBlockX, result.m_localBlockY, result.m_localBlockZ, 0);
			}
				break;
			case 1: // Right click
			{
				auto result = Physics::Raycast(*m_world->m_chunkManager, _camera->position, _camera->Front(), 10.0f);

				float distX = result.m_hitPos.x - (result.m_blockX + .5f);
				float distY = result.m_hitPos.y - (result.m_blockY + .5f);
				float distZ = result.m_hitPos.z - (result.m_blockZ + .5f);

				int blockX = result.m_blockX;
				int blockY = result.m_blockY;
				int blockZ = result.m_blockZ;

				// Choose face to place on
				if (abs(distX) > abs(distY) && abs(distX) > abs(distZ))
					blockX += (distX > 0 ? 1 : -1);
				else if (abs(distY) > abs(distX) && abs(distY) > abs(distZ))
					blockY += (distY > 0 ? 1 : -1);
				else
					blockZ += (distZ > 0 ? 1 : -1);

				int chunkX = blockX < 0 ? floorf(blockX / (float)CHUNK_SIZE) : blockX / (int)CHUNK_SIZE;
				int chunkY = blockY < 0 ? floorf(blockY / (float)CHUNK_SIZE) : blockY / (int)CHUNK_SIZE;
				int chunkZ = blockZ < 0 ? floorf(blockZ / (float)CHUNK_SIZE) : blockZ / (int)CHUNK_SIZE;

				int localBlockX = blockX - (chunkX * CHUNK_SIZE);
				int localBlockY = blockY - (chunkY * CHUNK_SIZE);
				int localBlockZ = blockZ - (chunkZ * CHUNK_SIZE);

				auto chunk = m_world->m_chunkManager->GetChunk(chunkX, chunkY, chunkZ);
				if (chunk == nullptr)
					return;

				uint16_t blockToReplace = chunk->GetBlockIdAtPos(localBlockX, localBlockY, localBlockZ);
				if (blockToReplace == 0 || Blocks::GetBlock(blockToReplace).blockType == Block::LIQUID)
					chunk->SetBlock(localBlockX, localBlockY, localBlockZ, _selectedBlock);
			}
				break;
			case 2: // Middle click
			{
				auto result = Physics::Raycast(*m_world->m_chunkManager, _camera->position, _camera->Front(), 10.0f);
				if (result.m_hit)
					_selectedBlock = result.m_chunk->GetBlockIdAtPos(result.m_localBlockX, result.m_localBlockY, result.m_localBlockZ);
			}
				break;
			}
		}

		void Render() override
		{
			if (_renderUI)
			{
				auto result = Physics::Raycast(*m_world->m_chunkManager, _camera->position, _camera->Front(), 10.0f);
				auto p = _camera->GetProjectionMatrix();
				auto v = _camera->GetViewMatrix();
				_renderingAPI->SetCullFace(false);
				_blockOutlineMesh->Render(v, p, { result.m_blockX, result.m_blockY, result.m_blockZ }, PolygonMode::Line);
				_renderingAPI->SetCullFace(true);
			}
		}

		void ConfigurePostProcessing() override
		{
			_underwaterShader->enabled = m_world->m_chunkManager->GetBlockIdAtPos(_camera->position) == 4;
		}

		void RenderUI() override
		{
			// FPS Calculations
			float fps = 1.0f / m_deltaTime;
			if (_lowestFps == -1 || fps < _lowestFps)
				_lowestFps = fps;
			if (_highestFps == -1 || fps > _highestFps)
				_highestFps = fps;
			_fpsCount++;
			std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(currentTimePoint - _fpsStartTime).count() >= 1)
			{
				_avgFps = _fpsCount;
				_lowestFps = -1;
				_highestFps = -1;
				_fpsCount = 0;
				_fpsStartTime = currentTimePoint;
			}

			ImGui::SetCurrentContext(GetImGuiContext());
			ImGui::Begin("Scuffed Minecraft", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("FPS: %d (%d / %d)", (int)_avgFps, (int)_lowestFps, (int)_highestFps);
			ImGui::Text("MS: %f", m_deltaTime * 100.0f);
			ImGui::Text("Position: x: %f, y: %f, z: %f", _camera->position.x, _camera->position.y, _camera->position.z);
			auto f = _camera->Front();
			ImGui::Text("Direction: x: %f, y: %f, z: %f", f.x, f.y, f.z);
			if (ImGui::SliderInt("Render Distance", &m_world->m_chunkManager->m_renderDistance, 0, 30))
				m_world->m_chunkManager->ClearChunkQueue();
			if (ImGui::SliderInt("Render Height", &m_world->m_chunkManager->m_renderHeight, 0, 10))
				m_world->m_chunkManager->ClearChunkQueue();
			ImGui::Checkbox("Use absolute Y axis for camera vertical movement", &_absoluteYMovement);
			if (ImGui::Checkbox("Vsync", &_vsync))
				_renderingAPI->SetVsync(_vsync);
			ImGui::Text("Selected Block: %s", Blocks::blocks[_selectedBlock].blockName);
			ImGui::End();

			_renderingAPI->SetCullFace(false);
			_renderingAPI->SetInvertRenderMode(true);
			_crosshairMesh->Render(glm::vec3(0, 0, -1.0f));
			_renderingAPI->SetInvertRenderMode(false);
			_renderingAPI->SetCullFace(true);
		}

		void CreateUIStuff()
		{
			// Crosshair
			glm::ivec2 size = _window->GetWindowSize();
			CrosshairVertex crosshairVertices[] = {
				{ { (float)size.x / 2 - 13.5, (float)size.y / 2 - 13.5 }, { 0.0f, 0.0f }},
				{ { (float)size.x / 2 + 13.5, (float)size.y / 2 - 13.5 }, { 1.0f, 0.0f }},
				{ { (float)size.x / 2 - 13.5, (float)size.y / 2 + 13.5 }, { 0.0f, 1.0f }},
				{ { (float)size.x / 2 + 13.5, (float)size.y / 2 + 13.5 }, { 1.0f, 1.0f }},
			};

			uint32_t crosshairIndices[] = {
				0, 2, 1,
				1, 2, 3
			};

			_crosshairTexture = _renderingAPI->CreateTexture("assets/sprites/crosshair.png");

			_crosshairShader = _renderingAPI->CreateShader("assets/shaders/sm/crosshair_vert.glsl", "assets/shaders/sm/crosshair_frag.glsl");

			glm::vec2 windowSize = _window->GetWindowSize();
			glm::mat4 ortho = glm::ortho(0.0f, windowSize.x, windowSize.y, 0.0f, 0.0f, 10.0f);
			_crosshairShader->Bind();
			_crosshairShader->SetMat4("projection", ortho);

			_crosshairMaterial = new CrosshairMaterial(_crosshairShader, _crosshairTexture);

			Mesh* crosshairMesh = _renderingAPI->CreateMesh();
			crosshairMesh->SetMesh(crosshairVertices, sizeof(CrosshairVertex), 4, crosshairIndices, 6);

			_crosshairMesh = new MeshRenderer(*_crosshairMaterial);
			_crosshairMesh->SetMesh(crosshairMesh, true);

			// Outline
			BlockOutlineVertex outlineVertices[] =
			{
				{ { -.001f, -.001f, -.001f } },  { { 1.001f, -.001f, -.001f } },
				{ { 1.001f, -.001f, -.001f } },  { { 1.001f, 1.001f, -.001f } },
				{ { 1.001f, 1.001f, -.001f } },  { { -.001f, 1.001f, -.001f } },
				{ { -.001f, 1.001f, -.001f } },  { { -.001f, -.001f, -.001f } },

				{ { -.001f, -.001f, -.001f } },  { { -.001f, -.001f, 1.001f } },
				{ { -.001f, -.001f, 1.001f } },  { { -.001f, 1.001f, 1.001f } },
				{ { -.001f, 1.001f, 1.001f } },  { { -.001f, 1.001f, -.001f } },
				{ { 1.001f, -.001f, -.001f } },  { { 1.001f, -.001f, 1.001f } },

				{ { 1.001f, -.001f, 1.001f } },  { { 1.001f, 1.001f, 1.001f } },
				{ { 1.001f, 1.001f, 1.001f } },  { { 1.001f, 1.001f, -.001f } },
				{ { -.001f, -.001f, 1.001f } },  { { 1.001f, -.001f, 1.001f } },
				{ { -.001f, 1.001f, 1.001f } },  { { 1.001f, 1.001f, 1.001f } },
			};

			uint32_t outlineIndices[] = {
				0, 1, 2, 3, 4, 5, 6, 7,
				8, 9, 10, 11, 12, 13, 14, 15,
				16, 17, 18, 19, 20, 21, 22, 23,
			};

			_blockOutlineShader = _renderingAPI->CreateShader("assets/shaders/sm/block_outline_vert.glsl", "assets/shaders/sm/block_outline_frag.glsl");
			_blockOutlineMaterial = new BlockOutlineMaterial(_blockOutlineShader);

			Mesh* blockOutlineMesh = _renderingAPI->CreateMesh();
			blockOutlineMesh->SetMesh(outlineVertices, sizeof(BlockOutlineVertex), 24, outlineIndices, 24);

			_blockOutlineMesh = new MeshRenderer(*_blockOutlineMaterial);
			_blockOutlineMesh->SetMesh(blockOutlineMesh, true);
		}

	private:
		int _fpsCount = 0;
		float _avgFps = 0;
		float _lowestFps = -1;
		float _highestFps = -1;
		std::chrono::steady_clock::time_point _fpsStartTime;
		bool _vsync = true;

		bool _paused = false;
		bool _firstFrame = true;
		float _moveSpeed = 10.0f;
		float _mouseSensitivity = 0.1f;
		bool _absoluteYMovement = false;

		int _selectedBlock = 1;

		Texture* _crosshairTexture;
		MeshRenderer* _crosshairMesh;
		MeshRenderer* _blockOutlineMesh;
		Shader* _crosshairShader;
		Shader* _blockOutlineShader;
		BaseMaterial* _crosshairMaterial;
		BaseMaterial* _blockOutlineMaterial;

		PostProcessingShader* _underwaterShader;
		Camera* _camera;
	};
}


WillowVox::Application* WillowVox::CreateApplication()
{
	return new ScuffedMinecraft::ScuffedMinecraft();
}