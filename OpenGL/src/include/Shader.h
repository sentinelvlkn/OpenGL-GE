#pragma once

/*
 * shader.h
 *
 *  Created on: Sep 27, 2025
 *      Author: volokin
 */

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class Shader{

public:
	unsigned int ID = 0;
	
	Shader() = default;

	Shader(const char* vertexPath, const char* fragmentPath);
	
	void use() const;

	//bool, int, float accordingly
	void setUniform(const std::string &name, bool value) const;
	void setUniform(const std::string &name, int value) const;
	void setUniform(const std::string &name, float value) const;
	
	//vec2
	void setUniform(const std::string &name, const glm::vec2 &value) const;

	//vec3
	void setUniform(const std::string &name, const glm::vec3 &value) const;

	//vec4
	void setUniform(const std::string &name, const glm::vec4 &value) const;

	//matrices (mat2, mat3, mat4 accordingly)
	void setUniform(const std::string &name, const glm::mat2 &mat) const;
	void setUniform(const std::string &name, const glm::mat3 &mat) const;
	void setUniform(const std::string &name, const glm::mat4 &mat) const;

private:
	static std::string readTextFile(const char* path);

	static unsigned int compileShader(GLenum type, const std::string& source, const char* debugName);

	static bool checkProgramLink(unsigned int program);

	GLint uniformLocation(const std::string& name) const;
};

