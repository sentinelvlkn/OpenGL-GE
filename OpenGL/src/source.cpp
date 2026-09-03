#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <include/Input.h>
#include "include/Shader.h"
#include "include/Model.h"
#include "include/AssetCache.h"
#include "include/FileSystem.h"
#include "include/TextRenderer.h"
#include "include/UiRenderer.h"
#include "include/DebugLog.h"
#include "include/DebugTerminal.h"
#include "include/FadeSystem.h"
#include "include/FramebufferSystem.h"
#include "include/ScreenRenderer.h"
#include "include/SceneRenderer.h"
#include "include/CameraSystem.h"
#include "include/GameplaySystem.h"
#include "include/GameUIRenderer.h"
#include "include/DebugInputSystem.h"
#include "include/AppState.h"
#include "include/GameState.h"
#include "include/AppContext.h"
#include "include/FrameTimingSystem.h"
#include "include/GameBootstrap.h"
#include "include/ShaderSystem.h"
#include <glm/glm.hpp>

#include <iostream>
#include "include/test_vertices.h"

constexpr int INITIAL_WIDTH = 800;
constexpr int INITIAL_HEIGHT = 600;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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

	AppContext appContext;
	appContext.appState = &appState;
	appContext.gameState = &gameState;

	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, &appContext);

	glfwSetErrorCallback(glfwErrorCallback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwGetFramebufferSize(window, &appState.framebufferWidth, &appState.framebufferHeight);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	appState.lastMouseX = appState.framebufferWidth * 0.5f;
	appState.lastMouseY = appState.framebufferHeight * 0.5f;
	appState.firstMouse = true;

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

	ShaderState shaders;
	initializeShaders(shaders);

	DebugLog::log("Debug terminal ready. Press F1 to toggle. Press F2 to print player state.");

	initializeGameState(gameState, "src/models/test_room/test_room.json");

	Model playerModel(
		FileSystem::pathFromRoot("src/models/test_character/test_character.obj").c_str()
	);

	updateRoomCamera(gameState);

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

	
	FramebufferState fboState;

	initializeFramebuffer(fboState, appState);
	
	initializeFrameTiming(appState);

	float promptScale = 0.75f;

	UiRenderer uiRenderer;

	DebugLog::log("[BOOT] Debug terminal initialized.");
	DebugLog::log("[BOOT] Press F1 to toggle terminal.");
	DebugLog::log("[BOOT] Press F2 to print player state.");

	//main game loop
	while (!glfwWindowShouldClose(window)) {

		updateFrameTiming(appState);

		std::string promptText;

		if (appState.framebufferResized)
			resizeFramebufferAttachments(fboState, appState);

		processInput(window, gameState);

		updateLightningDebugInput(window, appState, gameState);

		updateDebugInput(window, appState, gameState);

		updateFade(gameState, appState);

		updateRoomTransition(gameState);

		promptText = updateGameplay(window, appState, gameState);

		if (!gameState.roomTransition.transitioning)
			updateRoomCamera(gameState);

		renderSceneToFBO(fboState, shaders, playerModel, appState, gameState);

		renderScreen(quadVAO, shaders, fboState, appState, gameState);

		drawInteractionPrompt(
			promptText,
			uiRenderer,
			textRenderer,
			shaders,
			appState,
			gameState,
			promptScale
		);

		drawNoteOverlay(gameState, shaders, quadVAO);

		drawDebugTerminal(uiRenderer, textRenderer, shaders, appState);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	destroyFramebuffer(fboState);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
		
	glfwTerminate();
    return 0;
}	


void glfwErrorCallback(int error, const char* description){
	std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	if (width <= 0 || height <= 0)
		return;

	AppContext* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));

	if (!context || !context->appState)
		return;

	context->appState->framebufferWidth = width;
	context->appState->framebufferHeight = height;
	context->appState->framebufferResized = true;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	AppContext* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));

	if (!context || !context->appState || !context->gameState)
		return;

	AppState& appState = *context->appState;
	GameState& gameState = *context->gameState;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (appState.firstMouse) {
		appState.lastMouseX = xpos;
		appState.lastMouseY = ypos;
		appState.firstMouse = false;
	}

	float xoffset = xpos - appState.lastMouseX;
	float yoffset = appState.lastMouseY - ypos;

	appState.lastMouseX = xpos;
	appState.lastMouseY = ypos;

	gameState.camera.processMouseMovement(xoffset, yoffset);
	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	AppContext* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));

	if (!context || !context->gameState)
		return;

	context->gameState->camera.processMouseScroll(static_cast<float>(yoffset));
}




