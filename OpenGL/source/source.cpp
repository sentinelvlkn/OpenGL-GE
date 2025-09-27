

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

int WIDTH = 800;
int HEIGHT = 600;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	//window create
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OPENGL", NULL, NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	

	//load glad
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to load GLAD" << std::endl;
		return -1;
	}
	
	while(!glfwWindowShouldClose(window)){
		processInput(window);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	
	glfwTerminate();
    return 0;
}


void glfwErrorCallback(int error, const char* description){
	std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0,0,width,height);
}

void processInput(GLFWwindow* window){
if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	glfwSetWindowShouldClose(window, true);
}