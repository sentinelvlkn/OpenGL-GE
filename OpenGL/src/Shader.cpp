/*
 * shader.cpp
 *
 *  Created on: Sep 27, 2025
 *      Author: volokin
 */
#include "include/Shader.h"
//#include <unistd.h>
#include "filesystem"
#include "include/DebugLog.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath){
	
	//retrieve source code from the file paths	
	std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
	
	//ifstream exceptions
	vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	
	//reading files
	try {
		char cwd[1024];
		//getcwd(cwd, sizeof(cwd));
		//std::string cwd = std::filesystem::current_path().string();
		std::filesystem::path currentPath = std::filesystem::current_path();
		DebugLog::log("[SHADERS] Working dir: " + currentPath.string());

		DebugLog::log(std::string("[VERTEX SHADER] Reading: ") + vertexPath);
		vShaderFile.open(vertexPath);

		DebugLog::log(std::string("[FRAGMENT SHADER] Reading: ") + fragmentPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();	

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(std::ifstream::failure& e){
		DebugLog::log(std::string("[SHADER ERROR] Reading shader file failed: ") + e.what());
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	//compile shaders
	unsigned int vertex, fragment;
	
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	//shader program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	//deleting shaders, they are no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() const
{ 
    glUseProgram(ID); 
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec2(const std::string &name, float x, float y) const
{ 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, std::string type){
	GLint success;
	GLchar infoLog[1024];
	
	if(type != "PROGRAM"){
		
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				DebugLog::log(std::string("[SHADER COMPILATION ERROR] Type: ") + std::string(type) + "\n" + std::string(infoLog));
		}

	}else{

		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if(!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				DebugLog::log(std::string("[SHADER LINKING ERROR] Type: ") + std::string(type) + "\n" + std::string(infoLog));
		}

	}
}
