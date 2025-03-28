#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/BaseVertex.h>
#include <glm/glm.hpp>

namespace WillowVox
{
	class WILLOWVOX_API Vertex : public BaseVertex
	{
	public:
		Vertex(glm::vec3 position, glm::vec2 texPos)
			: m_position(position), m_texPos(texPos) {}
		Vertex(float x, float y, float z, float texX, float texY)
			: m_position({x,y,z}), m_texPos({texX,texY}) {}

		glm::vec3 m_position;
		glm::vec2 m_texPos;
	};
}