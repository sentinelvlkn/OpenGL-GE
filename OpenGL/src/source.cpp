
#include <cstddef>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "include/Shader.h"
#include "include/Camera.h"
#include "include/Model.h"
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
#include "include/test_vertices.h"

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
	//glfwWindowHint(GLFW_SAMPLES, 4);
	
	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	
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
	//glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Shader shader("src/shaders/vertex_shader.vert", "src/shaders/fragment_shader.frag");
	Shader skybShader ("src/shaders/skyb_vertex_shader.vert", "src/shaders/skyb_fragment_shader.frag");
	Shader screenShader("src/shaders/fbuffer_vertex_shader.vert", "src/shaders/fbuffer_fragment_shader.frag");
	Model backpackModel("src/models/backpack/backpack.obj");
	
	//skybox
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	
	//unsigned int cubeTexture = loadTexture("const char *path")
	
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER,quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	
	
//	//transparent VAO
//	unsigned int transparentVAO, transparentVBO;
//	glGenVertexArrays(1, &transparentVAO);
//	glGenBuffers(1, &transparentVBO);
//	glBindVertexArray(transparentVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
//	glEnableVertexAttribArray(2);
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
//	glBindVertexArray(0);
//	
//	stbi_set_flip_vertically_on_load(false);
//	unsigned int transparentTexture = loadTexture("src/models/transparent_tex/blending_transparent_window.png"); 
//	std::vector<glm::vec3> windows {
//        glm::vec3(1.0f, 0.0f, 1.0f),
//        glm::vec3( 1.5f, 0.0f, 0.51f),
//        glm::vec3( 0.0f, 0.0f, 0.7f),
//        glm::vec3(-0.3f, 0.0f, -2.3f),
//        glm::vec3 (0.5f, 0.0f, -0.6f)
//    };
	
//	unsigned int grassTexture = loadTexture("src/models/vegetation/grass.png");
//	std::vector<glm::vec3> grass_loc {
//        glm::vec3(3.0f, 0.0f, 1.0f),
//        glm::vec3( 4.5f, 0.0f, 0.51f),
//        glm::vec3( 2.0f, 0.0f, 0.7f),
//        glm::vec3(-2.3f, 0.0f, -2.3f),
//        glm::vec3 (7.5f, 0.0f, -0.6f)
//    };

	shader.use();	

	skybShader.use();
    skybShader.setInt("skybox", 0);
	
	screenShader.use();
	screenShader.setInt("screenTexture", 0);
	
	//framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	
	//color attachment texture
	unsigned int textureColorBuffer;
	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	//renderbuffer obj
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	//check if framebuffer if it was crated
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! Kaput!!!!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	while(!glfwWindowShouldClose(window)){
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		processInput(window);
		
//		std::map<float, glm::vec3> sorted;
//        for (unsigned int i = 0; i < windows.size(); i++)
//        {
//            float distance = glm::length(camera.Position - windows[i]);
//            sorted[distance] = windows[i];
//        }
		
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT , 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		
		//model, backpack currently
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
		
		//shader.use();		
		//windows
//		glBindVertexArray(transparentVAO);
//		glBindTexture(GL_TEXTURE_2D, transparentTexture);
//		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
//        {
//            model = glm::mat4(1.0f);
//            model = glm::translate(model, it->second);
//            shader.setMat4("model", model);
//            glDrawArrays(GL_TRIANGLES, 0, 6);
//        }
		
//		//grass
//		glBindTexture(GL_TEXTURE_2D, grassTexture);
//		for(unsigned int i=0; i<grass_loc.size(); i++){
//			model = glm::mat4(1.0f);
//			model = glm::translate(model, grass_loc[i]);
//			shader.setMat4("model", model);
//			glDrawArrays(GL_TRIANGLES, 0, 6);
//		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WIDTH, HEIGHT);		
		glDisable(GL_DEPTH_TEST); //disabled so screen-space quad isn't discarded
		 
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		screenShader.use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);	

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	
	
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteRenderbuffers(1, &RBO);
    glDeleteFramebuffers(1, &framebuffer);
		
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
	    if (firstMouse){
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.processMouseScroll(static_cast<float>(yoffset));
}
