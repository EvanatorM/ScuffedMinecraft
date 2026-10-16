#pragma once

#include <wv/pch.h>

namespace WillowVox
{
	class Window;

	class Camera
	{
	public:
		glm::vec3 position;
		glm::vec3 direction;
		float fov;

		Camera(glm::vec3 position = glm::vec3(0.0f), glm::vec3 direction = glm::vec3(0.0f, -90.0f, 0.0f), float fov = 70.0f);

		glm::vec3 Front() const;
		glm::vec3 Right() const;
		glm::vec3 Up() const;

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjMatrix() const;

	private:
		Window& _window;
	};
}