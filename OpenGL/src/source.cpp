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
#include "include/DebugInputSystem.h"
#include "include/AppState.h"
#include "include/GameState.h"
#include "glm/glm.hpp"
#include <vector>
#include <sstream>

#include <iostream>
#include "include/test_vertices.h"

constexpr int INITIAL_WIDTH = 800;
constexpr int INITIAL_HEIGHT = 600;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = INITIAL_WIDTH * 0.5;
float lastY = INITIAL_HEIGHT * 0.5;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	//window create
	GLFWwindow* window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "OPENGL", NULL, NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	AppState appState;
	GameState gameState;
	
	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, &appState);

	glfwSetErrorCallback(glfwErrorCallback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwGetFramebufferSize(window, &appState.framebufferWidth, &appState.framebufferHeight);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//load glad
	bool gladLoaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (!gladLoaded) {
		std::cout << "Failed to load GLAD" << std::endl;
		return -1;
	}

	std::cout << "GLAD load: OK\n";
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

	gameState.room = loadRoomFromJson(
		FileSystem::pathFromRoot("src/models/test_room/test_room.json").c_str()
	);

	gameState.roomModel = &getRoomModel(gameState.room);

	Model playerModel(
		FileSystem::pathFromRoot("src/models/test_character/test_character.obj").c_str()
	);

	gameState.player.position = gameState.room.playerSpawn;

	roomCameraSwitch(gameState.room, gameState.player.position);
	applyRoomCamera(gameState.room, gameState.camera);

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, appState.framebufferWidth, appState.framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	//renderbuffer obj
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appState.framebufferWidth, appState.framebufferHeight);
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

		deltaTime = glm::min(deltaTime, 1.0f / 30.0f);

		std::string promptText;

		if (appState.framebufferResized)
			resizeFramebufferAttachments(
				framebuffer,
				textureColorBuffer,
				RBO,
				appState
			);
		

		processInput(window, gameState.fade);

		updateLightningDebugInput(window, deltaTime, gameState.lightingDebug);

		updateDebugInput(
			window,
			appState,
			gameState
		);

		updateFade(
			gameState.fade,
			deltaTime
		);

		updateRoomTransition(
			gameState.roomTransition,
			gameState.fade,
			gameState.room,
			gameState.roomModel,
			gameState.player,
			gameState.camera
		);

		promptText = updateGameplay(
			window,
			gameState.room,
			gameState.player,
			gameState.camera,
			gameState.roomTransition,
			gameState.fade,
			gameState.interaction,
			deltaTime
		);

		if (!gameState.roomTransition.transitioning) {
			updateRoomCamera(
				gameState.room,
				gameState.player,
				gameState.camera
			);
		}

		renderSceneToFBO(
			framebuffer,
			shader,
			playerModel,
			appState,
			gameState,
			time
		);

		renderScreen(
			quadVAO,
			screenShader,
			textureColorBuffer,
			appState,
			gameState,
			time
		);

		drawInteractionPrompt(
			promptText,
			!gameState.interaction.messageOpen && !gameState.roomTransition.transitioning,
			uiRenderer,
			uiRectangleShader,
			textRenderer,
			textShader,
			appState.framebufferWidth,
			appState.framebufferHeight,
			promptScale
		);

		drawNoteOverlay(
			gameState.interaction,
			uiShader,
			quadVAO
		);

		drawDebugTerminal(
			uiRenderer,
			uiRectangleShader,
			textRenderer,
			textShader,
			appState
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

	AppState *appState = static_cast<AppState *>(glfwGetWindowUserPointer(window));

	if (!appState)
		return;

	appState->framebufferWidth = width;
	appState->framebufferHeight = height;
	appState->framebufferResized = true;
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




