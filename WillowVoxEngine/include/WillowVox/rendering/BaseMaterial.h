#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/Shader.h>
#include <glm/glm.hpp>
#include <stddef.h>

namespace WillowVox
{
	class WILLOWVOX_API BaseMaterial
	{
	public:
		BaseMaterial(Shader* shader);

		void Bind();

		virtual void SetVertexAttributes() = 0;
		void SetCameraShaderProperties(const glm::mat4& view, const glm::mat4& projection);
		void SetModelShaderProperties(const glm::vec3& model);

	protected:
		virtual void SetShaderProperties() = 0;

		Shader* _shader;
	};
}