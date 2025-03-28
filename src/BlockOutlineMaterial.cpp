#include <BlockOutlineMaterial.h>
#include <BlockOutlineVertex.h>

namespace ScuffedMinecraft
{
	BlockOutlineMaterial::BlockOutlineMaterial(Shader* shader)
		: BaseMaterial(shader)
	{
	}

	void BlockOutlineMaterial::SetVertexAttributes()
	{
		RenderingAPI::m_renderingAPI->SetVertexAttrib3f(0, sizeof(BlockOutlineVertex), offsetof(BlockOutlineVertex, m_position));
	}

	void BlockOutlineMaterial::SetShaderProperties()
	{
		RenderingAPI::m_renderingAPI->SetBlending(false);
	}
}