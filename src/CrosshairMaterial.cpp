#include <CrosshairMaterial.h>
#include <CrosshairVertex.h>

namespace ScuffedMinecraft
{
	CrosshairMaterial::CrosshairMaterial(Shader* shader, Texture* texture)
		: BaseMaterial(shader)
	{
		_texture = texture;
	}

	void CrosshairMaterial::SetVertexAttributes()
	{
		RenderingAPI::m_renderingAPI->SetVertexAttrib2f(0, sizeof(CrosshairVertex), offsetof(CrosshairVertex, m_position));
		RenderingAPI::m_renderingAPI->SetVertexAttrib2f(1, sizeof(CrosshairVertex), offsetof(CrosshairVertex, m_texPos));
	}

	void CrosshairMaterial::SetShaderProperties()
	{
		_texture->BindTexture(Texture::TEX00);
		RenderingAPI::m_renderingAPI->SetBlending(false);
	}
}