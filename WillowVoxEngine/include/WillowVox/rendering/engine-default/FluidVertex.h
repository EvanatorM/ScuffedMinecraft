#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/BaseVertex.h>
#include <glm/glm.hpp>

namespace WillowVox
{
	class WILLOWVOX_API FluidVertex : public BaseVertex
	{
	public:
		FluidVertex(char xPos, char yPos, char zPos, glm::vec2 texPos, char direction, char top)
			: m_x(xPos), m_y(yPos), m_z(zPos), m_texPos(texPos), m_direction(direction), m_top(top) {}
		FluidVertex(char xPos, char yPos, char zPos, float texX, float texY, char direction, char top)
			: m_x(xPos), m_y(yPos), m_z(zPos), m_texPos({texX,texY}), m_direction(direction), m_top(top) {}

		char m_x, m_y, m_z;
		glm::vec2 m_texPos;
		char m_direction;
		char m_top;
	};
}