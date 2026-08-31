#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <include/Input.h>
#include "include/Shader.h"
#include "include/Camera.h"
#include "include/Model.h"
#include "include/Room.h"
#include "include/Player.h"
#include "include/InteractionSystem.h"
#include "include/AssetCache.h"
#include "include/LightingSystem.h"
#include "include/FileSystem.h"
#include "include/TextRenderer.h"
#include "include/UiRenderer.h"
#include "include/DebugLog.h"
#include "include/DebugTerminal.h"
#include "include/FadeSystem.h"
#include "include/FramebufferSystem.h"
#include "include/ScreenRenderer.h"
#include "include/RoomTransitionSystem.h"
#include "include/CameraSystem.h"
#include "include/GameplaySystem.h"
#include "include/GameUIRenderer.h"
#include "glm/glm.hpp"
#include <vector>
#include <sstream>

#include <iostream>
#include "include/test_vertices.h"

int WIDTH = 800;
int HEIGHT = 600;

static int gFramebufferWidth = WIDTH;
static int gFramebufferHeight = HEIGHT;
static bool gFramebufferResized = false;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)gFramebufferWidth / 2.0;
float lastY = (float)gFramebufferHeight / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

static bool debugTerminalOpen = false;

static RoomTransitionState roomTransitionState;

//light debug 
static LightingDebugState lightingDebugState;

static FadeState fadeState;

static InteractionRuntimeState interactionState;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

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
	glfwGetFramebufferSize(window, &gFramebufferWidth, &gFramebufferHeight);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//load glad
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to load GLAD" << std::endl;
		return -1;
	}
	
	std::cout << "GLAD load: " << (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) ? "OK" : "FAIL")<< "\n";
	std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";
	
	glEnable(GL_DEPTH_TEST);
		
	Shader shader( 
		FileSystem::pathFromRoot("src/shaders/vertex_shader.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/fragment_shader.frag").c_str()
	);

	Shader screenShader(
		FileSystem::pathFromRoot("src/shaders/fbuffer_vertex_shader.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/fbuffer_fragment_shader.frag").c_str()
	);

	Shader uiShader(
		FileSystem::pathFromRoot("src/shaders/note_quad.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/note_quad.frag").c_str()
	);

	Shader textShader(
		FileSystem::pathFromRoot("src/shaders/text.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/text.frag").c_str()
	);

	Shader uiRectangleShader(
		FileSystem::pathFromRoot("src/shaders/ui_rectangle.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/ui_rectangle.frag").c_str()
	);

	DebugLog::log("Debug terminal ready. Press F1 to toggle. Press F2 to print player state.");

	Room testRoom = loadRoomFromJson(
		FileSystem::pathFromRoot("src/models/test_room/test_room.json").c_str()
	);


	for (int i = 0; i < testRoom.cameras.size(); ++i) {
    		auto& c = testRoom.cameras[i];
			{
				std::ostringstream ss;

				ss << "Cam " << i << " min(" << c.aabbMin.x << ","
				   << c.aabbMin.y << "," << c.aabbMin.z << ") "
				   << "max(" << c.aabbMax.x << "," << c.aabbMax.y << ","
				   << c.aabbMax.z << ")";

				DebugLog::log(ss.str());
			}
			
	}
	

	Model* testRoomModel = nullptr;
	testRoomModel = &getRoomModel(testRoom);

	Model playerModel(
		FileSystem::pathFromRoot("src/models/test_character/test_character.obj").c_str()
	);

	Player player;
	player.position = testRoom.playerSpawn;

	TextRenderer textRenderer;
	std::string fontPath = FileSystem::pathFromRoot("src/ui/fonts/LektonNerdFontMono-Regular.ttf");

	if (!textRenderer.loadFont(fontPath, 24))
		DebugLog::log("[TEXT ERROR] Failed to load the font: " + fontPath);

	
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

	screenShader.use();
	//screenShader.setInt("screenTexture", 0);
	screenShader.setFloat("vignetteStrength", 0.5f);
	screenShader.setFloat("grainStrength", 0.02f);
	screenShader.setFloat("saturation", 0.7f);
	screenShader.setFloat("contrast", 1.2f);

	uiShader.use();
	uiShader.setInt("uiTexture", 0);
	
	//framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	
	//color attachment texture
	unsigned int textureColorBuffer;
	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gFramebufferWidth, gFramebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	//renderbuffer obj
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gFramebufferWidth, gFramebufferHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	//check if framebuffer was crated
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		DebugLog::log("[FRAMEBUFFER ERROR] Framebuffer is not complete! Kaput!!!!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	lastFrame = static_cast<float>(glfwGetTime());

	float promptScale = 0.75f;

	UiRenderer uiRenderer;

	DebugLog::log("[BOOT] Debug terminal initialized.");
	DebugLog::log("[BOOT] Press F1 to toggle terminal.");
	DebugLog::log("[BOOT] Press F2 to print player state.");

	//main game loop
	while (!glfwWindowShouldClose(window)) {

    	float currentFrame = static_cast<float>(glfwGetTime());
    	deltaTime = currentFrame - lastFrame;
    	lastFrame = currentFrame;

		float time = static_cast<float>(glfwGetTime());

    	deltaTime = glm::min(deltaTime, 1.0f/30.0f);

		std::string promptText;

		if (gFramebufferResized) {
			resizeFramebufferAttachments(
				framebuffer,
				textureColorBuffer, 
				RBO, 
				gFramebufferWidth, 
				gFramebufferHeight
			);
			gFramebufferResized = false;
		}

    	processInput(window, fadeState);
		updateLightningDebugInput(window, deltaTime, lightingDebugState);

		if (keyPressedOnce(window, GLFW_KEY_F1))
			debugTerminalOpen = !debugTerminalOpen;

		if (keyPressedOnce(window, GLFW_KEY_F2))
			DebugLog::F2DebugPrint(testRoom, player);

    	updateFade(fadeState, deltaTime);

		updateRoomTransition(
			roomTransitionState,
			fadeState,
			testRoom,
			testRoomModel,
			player,
			camera
		);

    	// Gameplay only when not transitioning
		promptText = updateGameplay(
			window,
			testRoom,
			player,
			camera,
			roomTransitionState,
			fadeState,
			interactionState,
			deltaTime
		);

		if (!roomTransitionState.transitioning)
			updateRoomCamera(testRoom, player, camera);
    
   		static int lastCam = -1;
		if (testRoom.activeCamera != lastCam) {
			DebugLog::log("[CAMERA] Active camera: " + std::to_string(testRoom.activeCamera));
        	lastCam = testRoom.activeCamera;
		}

		renderSceneToFBO(framebuffer, 
			shader, 
			*testRoomModel, 
			playerModel, 
			player, 
			testRoom, 
			camera, 
			gFramebufferWidth, 
			gFramebufferHeight, 
			time, 
			lightingDebugState
		);

		renderScreen(
			quadVAO, 
			screenShader, 
			textureColorBuffer, 
			time, 
			fadeState.value, 
			gFramebufferWidth, 
			gFramebufferHeight
		);

		drawInteractionPrompt(
			promptText,
			!interactionState.messageOpen && !roomTransitionState.transitioning,
			uiRenderer,
			uiRectangleShader,
			textRenderer,
			textShader,
			gFramebufferWidth,
			gFramebufferHeight,
			promptScale
		);

		drawNoteOverlay(
			interactionState,
			uiShader,
			quadVAO
		);

		drawDebugTerminal(	
			uiRenderer, 
			uiRectangleShader,
			textRenderer, 
			textShader,
			gFramebufferWidth, 
			gFramebufferHeight,
			debugTerminalOpen					
		);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}	
	
	glDeleteVertexArrays(1, &quadVAO);
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
	if (width <= 0 || height <= 0)
		return;

	gFramebufferWidth = width;
	gFramebufferHeight = height;
	gFramebufferResized = true;
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




