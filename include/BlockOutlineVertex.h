#pragma once

#include <WillowVox/rendering/BaseVertex.h>
#include <glm/glm.hpp>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	class BlockOutlineVertex : public BaseVertex
	{
	public:
		BlockOutlineVertex(glm::vec3 position)
			: m_position(position) {}

		glm::vec3 m_position;
	};
}