#pragma once

#include <WillowVox/rendering/BaseVertex.h>
#include <glm/glm.hpp>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	class CrosshairVertex : public BaseVertex
	{
	public:
		CrosshairVertex(glm::vec2 position, glm::vec2 texPos)
			: m_position(position), m_texPos(texPos) {}
	
		glm::vec2 m_position;
		glm::vec2 m_texPos;
	};
}