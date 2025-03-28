#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <glm/glm.hpp>

namespace WillowVox
{
	class WILLOWVOX_API RawTriMaterial : public BaseMaterial
	{
	public:
		RawTriMaterial(Shader* shader, glm::vec4 color);

		void SetVertexAttributes() override;

	protected:
		void SetShaderProperties() override;

	private:
		glm::vec4 _color;
	};
}