/*
 * shader.cpp
 *
 *  Created on: Sep 27, 2025
 *      Author: volokin
 */

#include "include/Shader.h"

#include "filesystem"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include "include/DebugLog.h"

std::string Shader::readTextFile(const char* path){
	
	std::ifstream file(path);
	
	if (!file.is_open()) {
		DebugLog::log(std::string("[SHADER ERROR] Couldn't open: ") + path);
		return {};
	}

	std::ostringstream stream;
	stream << file.rdbuf();
	
	if(file.bad()) {
		DebugLog::log(std::string("[SHADER ERROR] Failed while reading: ") + path);
		return {};
	}

	return stream.str();
}

unsigned int Shader::compileShader(GLenum type, const std::string& source, const char* debugName){

	if (source.empty()) {
		DebugLog::log(std::string("[SHADER ERROR] Empty source: ") + debugName);
		return 0;
	}

	unsigned int shader = glCreateShader(type);
	
	const char* sourcePtr = source.c_str();
	
	glShaderSource(shader, 1, &sourcePtr, nullptr);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_TRUE)
		return shader;

	GLint logLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

	std::string log;

	if (logLength > 0) {
		log.resize(static_cast<std::size_t>(logLength));

		glGetShaderInfoLog(shader, logLength, nullptr, log.data());
	}

	DebugLog::log(std::string("[SHADER COMPILE ERROR] ") + debugName + "\n" + log);

	glDeleteShader(shader);

	return 0;

}

bool Shader::checkProgramLink(unsigned int program) {

	GLint success = GL_FALSE;

	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (success == GL_TRUE)
	        return true;

	GLint logLength = 0;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	std::string log;

	if (logLength > 0) {
		log.resize(static_cast<std::size_t>(logLength));

		glGetProgramInfoLog(program, logLength, nullptr, log.data());
    	}

	DebugLog::log(std::string("[SHADER LINK ERROR]\n") + log);

	return false;

}

Shader::Shader(const char* vertexPath, const char* fragmentPath){
	DebugLog::log(std::string("[VERTEX SHADER] Reading: ") + vertexPath);

	std::string vertexSource = readTextFile(vertexPath);

	DebugLog::log(std::string("[FRAGMENT SHADER] Reading: ") + fragmentPath);

	std::string fragmentSource = readTextFile(fragmentPath);

	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);

	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

	if (vertexShader == 0 || fragmentShader == 0) {
		if (vertexShader != 0) 
			glDeleteShader(vertexShader);

		if (fragmentShader != 0) 
			glDeleteShader(fragmentShader);

		return;
	}

	unsigned int program = glCreateProgram();
	
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	if (!checkProgramLink(program)) {
		glDeleteProgram(program);
		return;
	}

	ID = program;
}

void Shader::use() const { 

	if (ID != 0)
		glUseProgram(ID); 

}

void Shader::setUniform(const std::string &name, bool value) const
{         
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniform1i(location, value ? 1 : 0);
}

void Shader::setUniform(const std::string &name, int value) const
{ 
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniform1i(location, value);
}

void Shader::setUniform(const std::string &name, float value) const
{ 
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniform1f(location, value);
}

void Shader::setUniform(const std::string &name, const glm::vec2 &value) const
{ 
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniform2fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::vec3 &value) const
{ 
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::vec4 &value) const
{ 
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniform4fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::mat2 &mat) const
{
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setUniform(const std::string &name, const glm::mat3 &mat) const
{
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setUniform(const std::string &name, const glm::mat4 &mat) const
{
	GLint location = uniformLocation(name);
	if (location >= 0)
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}


GLint Shader::uniformLocation(const std::string& name ) const {

    if (ID == 0)
        return -1;

    return glGetUniformLocation(ID, name.c_str());

}
