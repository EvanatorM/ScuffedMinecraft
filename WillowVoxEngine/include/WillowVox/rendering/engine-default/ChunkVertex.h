#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/BaseVertex.h>
#include <glm/glm.hpp>

namespace WillowVox
{
	class WILLOWVOX_API ChunkVertex : public BaseVertex
	{
	public:
		ChunkVertex(char xPos, char yPos, char zPos, glm::vec2 texPos, char direction)
			: m_x(xPos), m_y(yPos), m_z(zPos), m_texPos(texPos), m_direction(direction) {}
		ChunkVertex(char xPos, char yPos, char zPos, float texX, float texY, char direction)
			: m_x(xPos), m_y(yPos), m_z(zPos), m_texPos({texX,texY}), m_direction(direction) {}

		char m_x, m_y, m_z;
		glm::vec2 m_texPos;
		char m_direction;
	};
}