#pragma once

#include <wv/pch.h>

namespace WillowVox
{
	class Window
	{
	public:
		Window(const std::string& name = "Vulkan", int width = 800, int height = 600);
		~Window();

		static Window* GetInstance() { return s_instance; }

		bool ShouldClose() const;

		void PollEvents();

		glm::ivec2 GetSize() const;
		double GetTime() const;

		void* GetNativeWindow() const;

		bool windowResized = false;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;

		static Window* s_instance;
	};
}