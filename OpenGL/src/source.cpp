
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "include/Shader.h"
#include "include/Camera.h"
#include "include/Model.h"
#include "include/skybVertices.h"
#include "stb_image.h"
#include "include/FileSystem.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <filesystem>

int WIDTH = 800;
int HEIGHT = 600;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)WIDTH / 2.0;
float lastY = (float)HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//load glad
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to load GLAD" << std::endl;
		return -1;
	}
	
	unsigned int cubemapTexture = loadCubemap(faces);
	stbi_set_flip_vertically_on_load(true);
	
	glEnable(GL_DEPTH_TEST);

	Shader shader("src/shaders/vertex_shader.vert", "src/shaders/fragment_shader.frag");
	Shader skybShader ("src/shaders/skyb_vertex_shader.vert", "src/shaders/skyb_fragment_shader.frag");
	Model backpackModel("src/models/backpack/backpack.obj");
		
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	
	//unsigned int cubeTexture = loadTexture("const char *path")
	
	
	skybShader.use();
    skybShader.setInt("skybox", 0);
	
	while(!glfwWindowShouldClose(window)){
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		processInput(window);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT , 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, static_cast<float>(glfwGetTime() * glm::radians(50.0f) * 2.0f), glm::vec3(0.1f, 1.0f, 0.1f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setMat4("model", model);
		backpackModel.Draw(shader);
		
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glDepthMask(GL_FALSE);
		
		skybShader.use();
        glm::mat4 skyView = glm::mat4(glm::mat3(camera.getViewMatrix())); // remove translation from the view matrix
        skybShader.setMat4("view", skyView);
        skybShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS); // set depth function back to default
		
		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	
	
	glDeleteVertexArrays(1, &skyboxVAO);
    
    glDeleteBuffers(1, &skyboxVBO);
	
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

if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	camera.processKeyboard(FORWARD, deltaTime);

if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	camera.processKeyboard(BACKWARD, deltaTime);

if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	camera.processKeyboard(LEFT, deltaTime);

if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	camera.processKeyboard(RIGHT, deltaTime);


}

unsigned int loadTexture(const char* path){ //load 2D texture from file
	unsigned int textureID;
	glGenTextures(1, &textureID);
	
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if(data){
		GLenum format;
		switch(nrComponents){
			case 1: format = GL_RED; break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
			default: break; 
		}
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	
	return textureID;
}

// loads a cubemap texture from 6 individual texture faces in order:
// +X (right) -X (left)
// +Y (top) -Y (bottom)
// +Z (front) -Z (back)
unsigned int loadCubemap(std::vector<std::string> faces){
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	for(unsigned int i=0; i<faces.size(); i++){
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if(data){
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return textureID;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	float xpos = static_cast<float>(xposIn);
	    float ypos = static_cast<float>(yposIn);
	    if (firstMouse)
	    {
	        lastX = xpos;
	        lastY = ypos;
	        firstMouse = false;
	    }

	    float xoffset = xpos - lastX;
	    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	    lastX = xpos;
	    lastY = ypos;

	    camera.processMouseMovement(xoffset, yoffset);
	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(static_cast<float>(yoffset));
}
