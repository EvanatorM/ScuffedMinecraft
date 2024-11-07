#pragma once

#include <string>
#include <glm/glm.hpp> // Used to pass vec or mat to GPU

class Shader
{
public:
	// the program ID
	unsigned int ID;

	// constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath);
	// use/activate the shader
	void use();
	// utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;

	// Overloaded uniform setter to avoid type error or overflows
	void setUniform(const std::string& uniformName, int val);
	void setUniform(const std::string& uniformName, unsigned int val);
	void setUniform(const std::string& uniformName, float val);
	void setUniform(const std::string& uniformName, double val);
	void setUniform(const std::string& uniformName, glm::vec2 val);
	void setUniform(const std::string& uniformName, glm::vec3 val);
	void setUniform(const std::string& uniformName, glm::ivec3 val);
	void setUniform(const std::string& uniformName, glm::mat3 val);
	void setUniform(const std::string& uniformName, glm::mat4 val);

	/// Returns -1 if the name is not valid
	int getUniformID(const std::string& name) const;

private:
	/// True if a opengl error was reported
	/// This method should actually be global or something similiar so that everyone can 
	/// use it but for now this is enough because there isn't even real error handling :)
	bool errorOccurd(const std::string& origin) const;
};