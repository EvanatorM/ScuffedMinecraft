#pragma once

#include <wv/pch.h>

namespace WillowVox
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 texCoord;

		Vertex(glm::vec3 pos, glm::vec2 texCoord)
			: pos(pos), texCoord(texCoord) {}
	};
}