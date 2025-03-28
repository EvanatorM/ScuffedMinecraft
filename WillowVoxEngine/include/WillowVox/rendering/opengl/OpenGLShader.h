#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/Shader.h>

namespace WillowVox
{
	class WILLOWVOX_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const char* vertexShaderPath, const char* fragmentShaderPath);
		OpenGLShader(const char* vertexShaderCode, const char* fragmentShaderCode, bool codePassed);
		~OpenGLShader();

		void Bind() override;

		void SetBool(const char* name, bool value) const override;
		void SetInt(const char* name, int value) const override;
		void SetFloat(const char* name, float value) const override;
		void SetVec2(const char* name, glm::vec2 value) const override;
		void SetVec2(const char* name, float x, float y) const override;
		void SetVec3(const char* name, glm::vec3 value) const override;
		void SetVec3(const char* name, float x, float y, float z) const override;
		void SetVec4(const char* name, glm::vec4 value) const override;
		void SetVec4(const char* name, float x, float y, float z, float w) const override;
		void SetMat4(const char* name, glm::mat4 value) const override;

	private:
		unsigned int _programId;
	};
}