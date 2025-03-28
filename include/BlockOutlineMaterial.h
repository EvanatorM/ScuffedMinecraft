#pragma once

#include <WillowVox/WillowVox.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <WillowVox/rendering/Texture.h>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	class BlockOutlineMaterial : public BaseMaterial
	{
	public:
		BlockOutlineMaterial(Shader* shader);

		void SetVertexAttributes() override;

	protected:
		void SetShaderProperties() override;
	};
}