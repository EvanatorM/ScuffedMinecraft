#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/WindowCloseEvent.h>
#include <WillowVox/events/WindowResizeEvent.h>
#include <WillowVox/events/MouseClickEvent.h>
#include <WillowVox/events/MouseReleaseEvent.h>
#include <WillowVox/events/MouseScrollEvent.h>
#include <WillowVox/events/MouseMoveEvent.h>
#include <WillowVox/events/KeyPressEvent.h>
#include <WillowVox/events/KeyReleaseEvent.h>
#include <WillowVox/events/EventDispatcher.h>
#include <WillowVox/input/Key.h>
#include <WillowVox/rendering/Shader.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace WillowVox
{
	struct WILLOWVOX_API PostProcessingShader
	{
		PostProcessingShader(Shader* shader, bool enabled = true)
			: shader(shader), enabled(enabled) {}

		Shader* shader;
		bool enabled;
	};

	class WILLOWVOX_API Window
	{
	public:
		// Actions
		virtual void FrameStart() = 0;
		virtual void PostProcessingStart() = 0;
		virtual void PostProcessingEnd() = 0;
		virtual void UIStart() = 0;
		virtual void UIEnd() = 0;
		virtual void FrameEnd() = 0;
		virtual void CloseWindow() = 0;

		// Set variables
		virtual void SetBackgroundColor(glm::vec4 color) = 0;
		virtual void SetBackgroundColor(float r, float g, float b, float a) = 0;

		virtual void SetMouseDisabled(bool state) = 0;
		virtual void ToggleMouseDisabled() = 0;

		void AddPostProcessingShader(PostProcessingShader* shader);

		// Get variables
		virtual bool ShouldClose() = 0;
		virtual glm::ivec2 GetWindowSize() = 0;
		virtual bool KeyDown(Key key) = 0;
		virtual bool MouseButtonDown(int button) = 0;
		virtual glm::vec2 GetMousePos() = 0;
		virtual bool MouseDisabled() = 0;

		// Events
		EventDispatcher<WindowCloseEvent> WindowCloseEventDispatcher;
		EventDispatcher<WindowResizeEvent> WindowResizeEventDispatcher;
		EventDispatcher<MouseScrollEvent> MouseScrollEventDispatcher;
		EventDispatcher<MouseClickEvent> MouseClickEventDispatcher;
		EventDispatcher<MouseReleaseEvent> MouseReleaseEventDispatcher;
		EventDispatcher<MouseMoveEvent> MouseMoveEventDispatcher;
		EventDispatcher<KeyPressEvent> KeyPressEventDispatcher;
		EventDispatcher<KeyReleaseEvent> KeyReleaseEventDispatcher;

	protected:
		std::vector<PostProcessingShader*> _postProcessingShaders;
	};
}