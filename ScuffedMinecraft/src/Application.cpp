#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
int fpsCount = 0;
std::chrono::steady_clock::time_point fpsStartTime;
float avgFps = 0;
float lowestFps = -1;
float highestFps = -1;
float lastX = 400, lastY = 300;
bool firstMouse = true;

bool menuMode = false;
bool escapeDown = false;

float windowX = 1920;
float windowY = 1080;

GLuint framebufferTexture;
GLuint depthTexture;

Camera camera;

// Window options
#define VSYNC 1 // 0 for off, 1 for on

float rectangleVertices[] =
{
	 // Coords     // TexCoords
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f, 1.0f,
};

int main()
{
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
	glfwSwapInterval(VSYNC);

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

	stbi_image_free(data);

	// Create camera
	camera = Camera(glm::vec3(0.0f, 25.0f, 0.0f));

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Planet::planet = new Planet(&shader, &waterShader, &billboardShader);

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

		// FPS Calculations
		float fps = 1.0f / deltaTime;
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

		// Input
		processInput(window);

		// Rendering
		glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

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
		unsigned int viewLoc = glGetUniformLocation(shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		unsigned int projectionLoc = glGetUniformLocation(shader.ID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		waterShader.use();
		viewLoc = glGetUniformLocation(waterShader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		projectionLoc = glGetUniformLocation(waterShader.ID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		billboardShader.use();
		viewLoc = glGetUniformLocation(billboardShader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		projectionLoc = glGetUniformLocation(billboardShader.ID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		Planet::planet->Update(camera.Position.x, camera.Position.y, camera.Position.z);

		framebufferShader.use();

		// Check if player is underwater
		int blockX = camera.Position.x < 0 ? camera.Position.x - 1 : camera.Position.x;
		int blockY = camera.Position.y < 0 ? camera.Position.y - 1 : camera.Position.y;
		int blockZ = camera.Position.z < 0 ? camera.Position.z - 1 : camera.Position.z;

		int chunkX = blockX < 0 ? floorf(blockX / (float)Planet::chunkSize) : blockX / (int)Planet::chunkSize;
		int chunkY = blockY < 0 ? floorf(blockY / (float)Planet::chunkSize) : blockY / (int)Planet::chunkSize;
		int chunkZ = blockZ < 0 ? floorf(blockZ / (float)Planet::chunkSize) : blockZ / (int)Planet::chunkSize;

		int localBlockX = blockX - (chunkX * Planet::chunkSize);
		int localBlockY = blockY - (chunkY * Planet::chunkSize);
		int localBlockZ = blockZ - (chunkZ * Planet::chunkSize);

		Chunk* chunk = Planet::planet->GetChunk(chunkX, chunkY, chunkZ);
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

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		// disabled so that the screen renders
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Draw ImGui UI
		ImGui::Begin("Test");
		ImGui::Text("FPS: %f (Avg: %f, Min: %f, Max: %f)", fps, avgFps, lowestFps, highestFps);
		ImGui::Text("MS: %f", deltaTime * 1000.f);
		ImGui::Text("Facing: %.1f/%.1f", camera.Pitch, camera.Yaw);
		ImGui::Text("Pos: %.2f, %.2f, %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
		ImGui::Text("Chunks: %d (%d rendered)", Planet::planet->numChunks, Planet::planet->numChunksRendered);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Check and call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		//std::cout << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << '\n';
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void framebufferSizeCallback(GLFWwindow*  window, int width, int height)
{
	windowX = width;
	windowY = height;
	glViewport(0, 0, width, height);
	// Resize the framebuffer texture
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	// resize depth buffer texture
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowX, windowY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}

void processInput(GLFWwindow* window)
{
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
	camera.ProcessMouseScroll(yoffset);
}