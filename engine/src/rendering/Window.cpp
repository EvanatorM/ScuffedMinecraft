#include <wv/rendering/Window.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <wv/Logger.h>

namespace WillowVox
{
	Window* Window::s_instance = nullptr;

	struct Window::Impl
	{
		GLFWwindow* window = nullptr;
	};

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		self->windowResized = true;
	}

	Window::Window(const std::string& name, int width, int height)
	{
		s_instance = this;

		_impl = std::make_unique<Impl>();

		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		
		_impl->window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(_impl->window, this);
		glfwSetFramebufferSizeCallback(_impl->window, framebufferResizeCallback);

		LOG_INFO("Window successfully created");
	}

	Window::~Window()
	{
		glfwDestroyWindow(_impl->window);

		glfwTerminate();

		LOG_INFO("GLFW terminated");
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(_impl->window);
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	glm::ivec2 Window::GetSize() const
	{
		int x, y;
		glfwGetFramebufferSize(_impl->window, &x, &y);
		return { x, y };
	}

	double Window::GetTime() const
	{
		return glfwGetTime();
	}

	void* Window::GetNativeWindow() const
	{
		return _impl->window;
	}
}