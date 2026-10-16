#include <wv/rendering/Camera.h>

#include <wv/rendering/Window.h>

namespace WillowVox
{
	Camera::Camera(glm::vec3 position, glm::vec3 direction, float fov)
		: position(position), direction(direction), fov(fov), _window(*Window::GetInstance())
	{
	}

	glm::vec3 Camera::Front() const
	{
		glm::vec3 front;
		front.x = cos(glm::radians(direction.y)) * cos(glm::radians(direction.x));
		front.y = sin(glm::radians(direction.x));
		front.z = sin(glm::radians(direction.y)) * cos(glm::radians(direction.x));
		return glm::normalize(front);
	}

	glm::vec3 Camera::Right() const
	{
		return glm::normalize(glm::cross(Front(), glm::vec3(0, 1, 0)));
	}

	glm::vec3 Camera::Up() const
	{
		return glm::normalize(glm::cross(Right(), Front()));
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		return glm::lookAt(position, position + Front(), Up());
	}

	glm::mat4 Camera::GetProjMatrix() const
	{
		glm::ivec2 w = _window.GetSize();
		return glm::perspective(glm::radians(fov), (float)w.x / (float)w.y, 0.1f, 1000.0f);
	}
}