#include "Shader.h"

#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "Error reading shader source files\n";
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// 2. compile shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, nullptr);
	glCompileShader(vertex);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		std::cout << "Error compiling vertex shader!\n" << infoLog << '\n';
	}

	// fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, nullptr);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		std::cout << "Error compiling fragment shader!\n" << infoLog << '\n';
	}

	// shader program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	// print linking errors if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, nullptr, infoLog);
		std::cout << "Error linking shader program!\n" << infoLog << '\n';
	}

	// delete the shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setUniform(const std::string& uniformName, int val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform1i(uLoc, val);
}
void Shader::setUniform(const std::string& uniformName, unsigned int val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform1i(uLoc, val);
}
void Shader::setUniform(const std::string& uniformName, float val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform1f(uLoc, val);
}
void Shader::setUniform(const std::string& uniformName, double val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform1f(uLoc, val);
}
void Shader::setUniform(const std::string& uniformName, glm::vec2 val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform2f(uLoc, val.x, val.y);
}
void Shader::setUniform(const std::string& uniformName, glm::vec3 val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform3f(uLoc, val.x, val.y, val.z);
}
void Shader::setUniform(const std::string& uniformName, glm::ivec3 val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniform3f(uLoc, val.x, val.y, val.z);
}
void Shader::setUniform(const std::string& uniformName, glm::mat3 val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniformMatrix3fv(uLoc, 1, GL_TRUE, &val[0][0]);
}
void Shader::setUniform(const std::string& uniformName, glm::mat4 val)
{
	int uLoc = getUniformID(uniformName);
	if (uLoc < 0)
		return;
	glUniformMatrix4fv(uLoc, 1, GL_TRUE, &val[0][0]);
}

int Shader::getUniformID(const std::string& uniformName) const
{
	int uLoc = glGetUniformLocation(ID, uniformName.c_str());
	if (errorOccurd("While fetching location for uniform with the name " + uniformName))
		return -1;
	return uLoc; // Is only callable if no error happens
}

bool Shader::errorOccurd(const std::string& origin) const
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::cerr << origin << " || A OpenGL error occurd with the code " << err;
		std::cerr << std::endl;

		return true;
	}
	return false;
}