#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/opengl/OpenGLAPI.h>
#include <WillowVox/rendering/Window.h>

namespace WillowVox
{
	class WILLOWVOX_API OpenGLWindow : public Window
	{
	public:
		OpenGLWindow(int width, int height, const char* title);
		~OpenGLWindow();

		// Actions
		void FrameStart() override;
		void PostProcessingStart() override;
		void PostProcessingEnd() override;
		void UIStart() override;
		void UIEnd() override;
		void FrameEnd() override;
		void CloseWindow() override;

		// Set variables
		void SetBackgroundColor(glm::vec4 color) override;
		void SetBackgroundColor(float r, float g, float b, float a) override;
		
		void SetMouseDisabled(bool state) override;
		void ToggleMouseDisabled() override;

		// Get variables
		bool ShouldClose() override;
		glm::ivec2 GetWindowSize() override;
		bool KeyDown(Key key) override;
		bool MouseButtonDown(int button) override;
		glm::vec2 GetMousePos() override;
		bool MouseDisabled() override;

	private:
		GLFWwindow* _window;

		GLuint _framebufferTexture;
		GLuint _depthTexture;
		unsigned int _fbo;
		unsigned int _postProcessVAO, _postProcessVBO;
		Shader* _postProcessShader;

		float _width, _height;

		float _lastMouseX, _lastMouseY;
	};
}