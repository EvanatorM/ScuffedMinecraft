#pragma once

#include <WillowVox/WillowVox.h>
#include <WillowVox/rendering/BaseMaterial.h>
#include <WillowVox/rendering/Texture.h>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	class CrosshairMaterial : public BaseMaterial
	{
	public:
		CrosshairMaterial(Shader* shader, Texture* texture);

		void SetVertexAttributes() override;

	protected:
		void SetShaderProperties() override;

	private:
		Texture* _texture;
	};
}