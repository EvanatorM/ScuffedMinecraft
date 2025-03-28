#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef LINUX
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "../ConfigLoader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#include "Shader.h"
#include "Camera.h"
#include "Planet.h"
#include "Blocks.h"
#include "Physics.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void draw_hotbar_box(int index, void* texture);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
int fpsCount = 0;
int fpsCap = 240;
std::chrono::steady_clock::time_point fpsStartTime;
float avgFps = 0;
float lowestFps = -1;
float highestFps = -1;
float lastX = 400, lastY = 300;
bool firstMouse = true;

bool menuMode = false;
bool escapeDown = false;
bool f1Down = false;

// Window settings
float windowX = 1920;
float windowY = 1080;
float oldWindowX = 1920;
float oldWindowY = 1080;
int windowedXPos, windowedYPos;
bool vsync = true;
bool fullscreen = false;
bool prevFullscreen = false;

int selectedSlot = 0;
uint16_t blocksInEquipment[] = 
{
	0, 0, 0,
	0, 0, 0,
	0, 0, 0
};

bool uiEnabled = true;

Camera camera;

GLuint framebufferTexture;
GLuint depthTexture;

float rectangleVertices[] =
{
	 // Coords     // TexCoords
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};

float outlineVertices[] = 
{
	-.001f, -.001f, -.001f,  1.001f, -.001f, -.001f,
    1.001f, -.001f, -.001f,  1.001f, 1.001f, -.001f,
	1.001f, 1.001f, -.001f,  -.001f, 1.001f, -.001f,
	-.001f, 1.001f, -.001f,  -.001f, -.001f, -.001f,
	   		   		   							
	-.001f, -.001f, -.001f,  -.001f, -.001f, 1.001f,
	-.001f, -.001f, 1.001f,  -.001f, 1.001f, 1.001f,
	-.001f, 1.001f, 1.001f,  -.001f, 1.001f, -.001f,
	   		   		   							
	1.001f, -.001f, -.001f,  1.001f, -.001f, 1.001f,
	1.001f, -.001f, 1.001f,  1.001f, 1.001f, 1.001f,
	1.001f, 1.001f, 1.001f,  1.001f, 1.001f, -.001f,
	   		   		   							
	-.001f, -.001f, 1.001f,  1.001f, -.001f, 1.001f,
	-.001f, 1.001f, 1.001f,  1.001f, 1.001f, 1.001f,
};

float crosshairVertices[] =
{
	windowX / 2 - 13.5, windowY / 2 - 13.5,  0.0f, 0.0f,
	windowX / 2 + 13.5, windowY / 2 - 13.5,  1.0f, 0.0f,
	windowX / 2 + 13.5, windowY / 2 + 13.5,  1.0f, 1.0f,
				  					 
	windowX / 2 - 13.5, windowY / 2 - 13.5,  0.0f, 0.0f,
	windowX / 2 - 13.5, windowY / 2 + 13.5,  0.0f, 1.0f,
	windowX / 2 + 13.5, windowY / 2 + 13.5,  1.0f, 1.0f,
};

int main(int argc, char *argv[])
{
#ifdef LINUX
	char* resolved_path = realpath(argv[0], NULL);
	if (resolved_path == NULL) {
		printf("%s: Please do not place binary in PATH\n", argv[0]);
		exit(1);
	}
	size_t resolved_length = strlen(resolved_path);
	// remove executable from path
	for (size_t i = resolved_length; i > 0; i--) {
		if (resolved_path[i] == '/' && resolved_path[i + 1] != 0) {
			resolved_path[i + 1] = 0;
			resolved_length = i;
			break;
		}
	}
	char* assets_path = (char*)malloc(resolved_length + strlen("assets") + 2);
	strcpy(assets_path, resolved_path);
	strcpy(assets_path + resolved_length + 1, "assets");
	struct stat path_stat;
	if (stat(assets_path, &path_stat) == -1 || !S_ISDIR(path_stat.st_mode)) {
		printf("%s: Asset directory not found\n", argv[0]);
		exit(1);
	}
	free(assets_path);

	chdir(resolved_path);
	free(resolved_path);
#endif
	// Init ConfigLoader
	ConfigLoader::InitializeConfigMap();
	ConfigLoader::LoadConfig();

	// Clear configMap that was initialised to save on ressources ig?
	if (!ConfigLoader::bUseCustomConfiguration == true)
	{
		std::cout << "bUseCustomConfiguration is set to " << ConfigLoader::bUseCustomConfiguration << ", ignoring all Custom Configurations from Config.ini!" << std::endl;
		ConfigLoader::configMap.clear();
	}

	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	// Create window
	GLFWwindow* window = glfwCreateWindow(windowX, windowY, "Scuffed Minecraft", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(vsync ? 1 : 0);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	// Configure viewport and rendering
	glViewport(0, 0, windowX, windowY);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);

	// Create shaders
	Shader shader("assets/shaders/main_vert.glsl", "assets/shaders/main_frag.glsl");
	shader.use();

	shader.setFloat("texMultiplier", 0.0625f);

	Shader waterShader("assets/shaders/water_vert.glsl", "assets/shaders/water_frag.glsl");
	waterShader.use();

	waterShader.setFloat("texMultiplier", 0.0625f);

	Shader billboardShader("assets/shaders/billboard_vert.glsl", "assets/shaders/billboard_frag.glsl");
	billboardShader.use();

	billboardShader.setFloat("texMultiplier", 0.0625f);

	Shader framebufferShader("assets/shaders/framebuffer_vert.glsl", "assets/shaders/framebuffer_frag.glsl");

	Shader outlineShader("assets/shaders/block_outline_vert.glsl", "assets/shaders/block_outline_frag.glsl");

	Shader crosshairShader("assets/shaders/crosshair_vert.glsl", "assets/shaders/crosshair_frag.glsl");

	// Create post-processing framebuffer
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowX, windowY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowX, windowY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << '\n';

	unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	framebufferShader.use();
	glUniform1i(glGetUniformLocation(framebufferShader.ID, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(framebufferShader.ID, "depthTexture"), 1);

	unsigned int outlineVAO, outlineVBO;
	glGenVertexArrays(1, &outlineVAO);
	glGenBuffers(1, &outlineVBO);
	glBindVertexArray(outlineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(outlineVertices), &outlineVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	unsigned int crosshairVAO, crosshairVBO;
	glGenVertexArrays(1, &crosshairVAO);
	glGenBuffers(1, &crosshairVBO);
	glBindVertexArray(crosshairVAO);
	glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), &crosshairVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create terrain texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	 
	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load Texture
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("assets/sprites/block_map.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture\n";
	}
	ImTextureID block_map = (ImTextureID)texture;

	stbi_image_free(data);

	// Create crosshair texture
	unsigned int crosshairTexture;
	glGenTextures(1, &crosshairTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, crosshairTexture);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load Crosshair Texture
	unsigned char* data2 = stbi_load("assets/sprites/crosshair.png", &width, &height, &nrChannels, 0);
	if (data2)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture\n";
	}

	stbi_image_free(data2);

	// Create camera
	camera = Camera(glm::vec3(0.0f, 25.0f, 0.0f));

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Planet::planet = new Planet(&shader, &waterShader, &billboardShader);

	glm::mat4 ortho = glm::ortho(0.0f, (float)windowX, (float)windowY, 0.0f, 0.0f, 10.0f);

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	fpsStartTime = std::chrono::steady_clock::now();

	while (!glfwWindowShouldClose(window))
	{
		// Delta Time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float desiredDeltaTime = 1.0f / fpsCap;
		float correction = 0.0f;
		if (desiredDeltaTime > deltaTime) 
		{
			correction = desiredDeltaTime - deltaTime;
		};

		// FPS Correction
		std::this_thread::sleep_for(std::chrono::duration<float>(correction));

		// FPS Calculations
		float fps = 1.0f / (deltaTime + correction);
		if (lowestFps == -1 || fps < lowestFps)
			lowestFps = fps;
		if (highestFps == -1 || fps > highestFps)
			highestFps = fps;
		fpsCount++;
		std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(currentTimePoint - fpsStartTime).count() >= 1)
		{
			avgFps = fpsCount;
			lowestFps = -1;
			highestFps = -1;
			fpsCount = 0;
			fpsStartTime = currentTimePoint;
		}

		waterShader.use();
		waterShader.setFloat("time", currentFrame);
		outlineShader.use();
		outlineShader.setFloat("time", currentFrame);

		// Input
		processInput(window);

		// Rendering
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// New ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Projection matrix
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), windowX / windowY, 0.1f, 1000.0f);

		shader.use();
		shader.setMat4x4f( "view", view );
		shader.setMat4x4f( "projection", projection );

		waterShader.use();
		waterShader.setMat4x4f( "view", view );
		waterShader.setMat4x4f( "projection", projection );

		billboardShader.use();
		billboardShader.setMat4x4f( "view", view );
		billboardShader.setMat4x4f( "projection", projection );

		outlineShader.use();
		outlineShader.setMat4x4f( "view", view );
		outlineShader.setMat4x4f( "projection", projection );

		Planet::planet->Update(camera.Position);

		// -- Render block outline -- //
		if (uiEnabled)
		{
			// Get block position
			auto result = Physics::Raycast(camera.Position, camera.Front, 5);
			if (result.hit)
			{
				outlineShader.use();

				// Set outline view to position
				unsigned int modelLoc = glGetUniformLocation(outlineShader.ID, "model");
				glUniform3f(modelLoc, result.blockX, result.blockY, result.blockZ);

				// Render
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				//glEnable(GL_COLOR_LOGIC_OP);
				glLogicOp(GL_INVERT);
				glDisable(GL_CULL_FACE);
				glBindVertexArray(outlineVAO);
				glLineWidth(2.0);
				glDrawArrays(GL_LINES, 0, 24);

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_CULL_FACE);
				//glDisable(GL_COLOR_LOGIC_OP);
			}
		}

		framebufferShader.use();

		// -- Post Processing Stuff -- //

		// Check if player is underwater
		int blockX = camera.Position.x < 0 ? camera.Position.x - 1 : camera.Position.x;
		int blockY = camera.Position.y < 0 ? camera.Position.y - 1 : camera.Position.y;
		int blockZ = camera.Position.z < 0 ? camera.Position.z - 1 : camera.Position.z;

		int chunkX = blockX < 0 ? floorf(blockX / (float)CHUNK_SIZE) : blockX / (int)CHUNK_SIZE;
		int chunkY = blockY < 0 ? floorf(blockY / (float)CHUNK_SIZE) : blockY / (int)CHUNK_SIZE;
		int chunkZ = blockZ < 0 ? floorf(blockZ / (float)CHUNK_SIZE) : blockZ / (int)CHUNK_SIZE;

		int localBlockX = blockX - (chunkX * CHUNK_SIZE);
		int localBlockY = blockY - (chunkY * CHUNK_SIZE);
		int localBlockZ = blockZ - (chunkZ * CHUNK_SIZE);

		Chunk* chunk = Planet::planet->GetChunk(ChunkPos(chunkX, chunkY, chunkZ));
		if (chunk != nullptr)
		{
			unsigned int blockType = chunk->GetBlockAtPos(
				localBlockX,
				localBlockY,
				localBlockZ);

			if (Blocks::blocks[blockType].blockType == Block::LIQUID)
			{
				framebufferShader.setBool("underwater", true);
			}
			else
			{
				framebufferShader.setBool("underwater", false);
			}
		}

		// Post Processing
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (uiEnabled)
		{
			// -- Render Crosshair -- //

			// Render
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, crosshairTexture);

			crosshairShader.use();
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glEnable(GL_COLOR_LOGIC_OP);

			crosshairShader.setMat4x4f( "projection", ortho );
			glBindVertexArray(crosshairVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDisable(GL_COLOR_LOGIC_OP);

			// Draw ImGui UI
			ImGui::Begin("Test", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("FPS: %d (Avg: %d, Min: %d, Max: %d)", (int)fps, (int)avgFps, (int)lowestFps, (int)highestFps);
			ImGui::Text("MS: %f", (deltaTime + correction) * 100.0f);
			ImGui::InputInt("FPS Cap", &fpsCap);
			if (ImGui::Checkbox("VSYNC", &vsync))
				glfwSwapInterval(vsync ? 1 : 0);
			ImGui::Text("Chunks: %d (%d rendered)", Planet::planet->numChunks, Planet::planet->numChunksRendered);
			ImGui::Text("Position: x: %f, y: %f, z: %f", camera.Position.x, camera.Position.y, camera.Position.z);
			ImGui::Text("Direction: x: %f, y: %f, z: %f", camera.Front.x, camera.Front.y, camera.Front.z);
			ImGui::Text("Selected Block: %s", Blocks::blocks[blocksInEquipment[selectedSlot]].blockName.c_str());
			if (ImGui::SliderInt("Render Distance", &Planet::planet->renderDistance, 0, 30))
				Planet::planet->ClearChunkQueue();
			if (ImGui::SliderInt("Render Height", &Planet::planet->renderHeight, 0, 10))
				Planet::planet->ClearChunkQueue();
			ImGui::Checkbox("Use absolute Y axis for camera vertical movement", &camera.absoluteVerticalMovement);
			ImGui::Checkbox("Fullscreen", &fullscreen);
			ImGui::End();

			// Draw Hotbar
			ImGui::SetNextWindowPos(ImVec2(windowX / 2, windowY - windowY * 0.1f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(9 * 100, 100));
			ImGui::SetNextWindowBgAlpha(0.2f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 6);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Hotbar", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			for (int i = 0; i < 9; i++)
			{
				draw_hotbar_box(i, block_map);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 100);
			}
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
		}
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Toggling Fullscreen
		if (fullscreen != prevFullscreen)
		{
			if (fullscreen)
			{
				oldWindowX = windowX;
				oldWindowY = windowY;
				glfwGetWindowPos(window, &windowedXPos, &windowedYPos);
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			}
			else 
			{
				windowX = oldWindowX;
				windowY = oldWindowY;
				glfwSetWindowMonitor(window, nullptr, windowedXPos, windowedYPos, windowX, windowY, 0);
			}
			prevFullscreen = !prevFullscreen;
		};

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Check and call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		//std::cout << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << '\n';
	}

	delete Planet::planet;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void draw_hotbar_box(int index, void* texture)
{
	ImU32 colors = IM_COL32(80, 80, 80, 100);
	float size = 100.0f;

	Block block = Blocks::blocks[blocksInEquipment[index]];

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	
	ImVec2 p_min = ImGui::GetCursorScreenPos();
	ImVec2 p_max = ImVec2(p_min.x + size, p_min.y + size);

	if (index == selectedSlot)
	{
		draw_list->AddRectFilled(p_min, p_max, colors);
	}

	draw_list->AddRect(p_min, p_max, colors);

	float atlas = 256.0f;

	ImVec2 imageMin = ImVec2(p_min.x + 10, p_min.y + 10);
	ImVec2 imageMax = ImVec2(p_max.x - 10, p_max.y - 10);
	if (block.blockType == Block::BILLBOARD) 
	{
		ImVec2 uvSideMax = ImVec2(block.sideMinX * 16 / atlas, block.sideMinY * 16 / atlas);
		ImVec2 uvSideMin = ImVec2(block.sideMaxX * 16 / atlas, block.sideMaxY * 16 / atlas);
		draw_list->AddImage(texture, imageMin, imageMax, uvSideMin, uvSideMax);
	}
	else if (block.blockType == Block::SOLID || block.blockType == Block::LEAVES)
	{
		ImVec2 uvSideMin = ImVec2(block.sideMinX * 16 / atlas, block.sideMinY * 16 / atlas);
		ImVec2 uvSideMax = ImVec2(block.sideMaxX * 16 / atlas, block.sideMaxY * 16 / atlas);
		ImVec2 uvTopMin = ImVec2(block.topMaxX * 16 / atlas, block.topMaxY * 16 / atlas);
		ImVec2 uvTopMax = ImVec2(block.topMinX * 16 / atlas, block.topMinY * 16 / atlas);
		float cubeSize = size * 0.4f;
		float offsetY = cubeSize * 0.5f;
		float padX = size * 0.2f;
		float padY = size * 0.2f;

		ImVec2 topFrontLeft  = ImVec2(p_min.x + 0.75f * cubeSize + padX, p_min.y + padY);
		ImVec2 topFrontRight = ImVec2(p_min.x + 1.5f * cubeSize + padX, p_min.y + offsetY / 2 + padY);
		ImVec2 topBackLeft   = ImVec2(p_min.x + padX, p_min.y + offsetY / 2 + padY);
		ImVec2 topBackRight  = ImVec2(p_min.x + 0.75f * cubeSize + padX, p_min.y + offsetY + padY);
	
		// ImVec2 bottomFrontLeft  = ImVec2(p_min.x + 0.75f * cubeSize  + padX, p_min.y + cubeSize + padY);
		ImVec2 bottomFrontRight = ImVec2(p_min.x + 1.5f * cubeSize + padX, p_min.y + cubeSize + offsetY / 2 + padY);
		ImVec2 bottomBackLeft   = ImVec2(p_min.x + padX, p_min.y + cubeSize + offsetY / 2 + padY);
		ImVec2 bottomBackRight  = ImVec2(p_min.x + 0.75f * cubeSize + padX, p_min.y + cubeSize + offsetY + padY);
		
		ImU32 shadow = IM_COL32(0, 0, 0, 100);

		// Draw Top
		draw_list->AddImageQuad(texture, topBackLeft, topBackRight, topFrontRight, topFrontLeft, uvTopMin, 
			ImVec2(uvTopMax.x, uvTopMin.y), uvTopMax, ImVec2(uvTopMin.x, uvTopMax.y));

		// Draw Left
		draw_list->AddImageQuad(texture, bottomBackLeft, bottomBackRight, topBackRight, topBackLeft, uvSideMin, 
			ImVec2(uvSideMax.x, uvSideMin.y), uvSideMax, ImVec2(uvSideMin.x, uvSideMax.y));
		
		// Draw Right
		draw_list->AddImageQuad(texture, bottomBackRight, bottomFrontRight, topFrontRight, topBackRight,  uvSideMin, 
			ImVec2(uvSideMax.x, uvSideMin.y), uvSideMax, ImVec2(uvSideMin.x, uvSideMax.y));
		draw_list->AddQuadFilled(ImVec2(bottomBackRight.x, bottomBackRight.y - 0.75f), ImVec2(bottomFrontRight.x, bottomFrontRight.y - 0.75f), 
			topFrontRight, topBackRight, shadow);
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	windowX = width;
	windowY = height;
	glViewport(0, 0, width, height);

	// resize framebuffer texture
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowX, windowY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	// resize framebuffer depth texture
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowX, windowY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}

void processInput(GLFWwindow* window)
{
	// Change Item
	int buttons[] = 
	{
		GLFW_KEY_1,
		GLFW_KEY_2,
		GLFW_KEY_3,
		GLFW_KEY_4,
		GLFW_KEY_5,
		GLFW_KEY_6,
		GLFW_KEY_7,
		GLFW_KEY_8,
		GLFW_KEY_9,
	};
	for (int i = 0; i < 9; i++)
	{
		if (glfwGetKey(window, buttons[i]) == GLFW_PRESS)
		{
			selectedSlot = i;
		}
	};

	// Pause
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		if (escapeDown)
			return;

		escapeDown = true;
		menuMode = !menuMode;
		glfwSetInputMode(window, GLFW_CURSOR, menuMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		firstMouse = true;
		//glfwSetWindowShouldClose(window, true);
	}
	else
		escapeDown = false;

	// UI Toggle
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		if (f1Down)
			return;

		f1Down = true;
		uiEnabled = !uiEnabled;
	}
	else
		f1Down = false;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD_NO_Y, deltaTime);
		else
			camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (menuMode) return;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		auto result = Physics::Raycast(camera.Position, camera.Front, 5);
		if (!result.hit)
			return;

		result.chunk->UpdateBlock(result.localBlockX, result.localBlockY, result.localBlockZ, 0);
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
	{
		auto result = Physics::Raycast(camera.Position, camera.Front, 5);
		if (!result.hit)
			return;

		blocksInEquipment[selectedSlot] = result.chunk->GetBlockAtPos(result.localBlockX, result.localBlockY, result.localBlockZ);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		auto result = Physics::Raycast(camera.Position, camera.Front, 5);
		if (!result.hit)
			return;

		float distX = result.hitPos.x - (result.blockX + .5f);
		float distY = result.hitPos.y - (result.blockY + .5f);
		float distZ = result.hitPos.z - (result.blockZ + .5f);

		int blockX = result.blockX;
		int blockY = result.blockY;
		int blockZ = result.blockZ;
		
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

		Chunk* chunk = Planet::planet->GetChunk(ChunkPos(chunkX, chunkY, chunkZ));
		uint16_t blockToReplace = chunk->GetBlockAtPos(localBlockX, localBlockY, localBlockZ);
		if (chunk != nullptr && (blockToReplace == 0 || Blocks::blocks[blockToReplace].blockType == Block::LIQUID))
		chunk->UpdateBlock(localBlockX, localBlockY, localBlockZ, blocksInEquipment[selectedSlot]);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (menuMode)
		return;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) 
	{
		camera.ProcessMouseScroll(yoffset);
	} 
	else 
	{
		selectedSlot -= yoffset;
		if (selectedSlot > 8)
		{
			selectedSlot = 0;
		}
		else if (selectedSlot < 0)
		{
			selectedSlot = 8;
		}
	}
}