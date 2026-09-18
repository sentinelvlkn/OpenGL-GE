#include "include/WindowSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "include/AppContext.h"
#include "include/AppState.h"
#include "include/GameState.h"

namespace {
void glfwErrorCallback(int error, const char *description) {
	std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	if (width <= 0 || height <= 0)
		return;

	AppContext *context =
		static_cast<AppContext *>(glfwGetWindowUserPointer(window));

	if (!context || !context->appState)
		return;

	context->appState->framebufferWidth = width;
	context->appState->framebufferHeight = height;
	context->appState->framebufferResized = true;
}

/*
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
	AppContext *context =
		static_cast<AppContext *>(glfwGetWindowUserPointer(window));

	if (!context || !context->appState || !context->gameState)
		return;

	AppState &appState = *context->appState;
	GameState &gameState = *context->gameState;

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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	AppContext *context =
		static_cast<AppContext *>(glfwGetWindowUserPointer(window));

	if (!context || !context->gameState)
		return;

	context->gameState->camera.processMouseScroll(static_cast<float>(yoffset));
}
*/

}

bool initializeWindow(
                        WindowState& windowState,
                        AppState& appState,
                        GameState& gameState,
                        AppContext& appContext,
                        int width,
                        int height,
                        const char* title,
                        bool enableMouseLook) {
	
	glfwSetErrorCallback(glfwErrorCallback);

	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	windowState.handle = glfwCreateWindow(
        	width,
	        height,
        	title,
	        nullptr,
	        nullptr
    	);

	if (windowState.handle == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	appContext.appState = &appState;
	appContext.gameState = &gameState;

	glfwMakeContextCurrent(windowState.handle);
	glfwSetWindowUserPointer(windowState.handle, &appContext);

	glfwSetFramebufferSizeCallback(windowState.handle, framebuffer_size_callback);
	glfwGetFramebufferSize(windowState.handle, &appState.framebufferWidth, &appState.framebufferHeight);

	int windowW = 0;
	int windowH = 0;

	glfwGetWindowSize(
		windowState.handle,
		&windowW,
		&windowH
	);

	appState.lastMouseX = windowW * 0.5f;
	appState.lastMouseY = windowH * 0.5f;
	appState.firstMouse = true;

	glfwSetInputMode(windowState.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
	bool gladLoaded = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

	if (!gladLoaded) {
		std::cout << "Failed to load GLAD" << std::endl;
		glfwDestroyWindow(windowState.handle);
		windowState.handle = nullptr;
		glfwTerminate();
		return false;
	}


	std::cout << "GLAD load: OK\n";
	std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";

	glEnable(GL_DEPTH_TEST);

	return true;

}

bool shouldWindowClose(const WindowState& windowState) {
	if (!windowState.handle)
		return true;

	return glfwWindowShouldClose(windowState.handle);
}

void swapWindowBuffers(const WindowState& windowState) {
	if (!windowState.handle)
		return;

	glfwSwapBuffers(windowState.handle);
}


void pollWindowEvents() {
    glfwPollEvents();
}

void shutdownWindow(WindowState& windowState) {
    if (windowState.handle) {
        glfwDestroyWindow(windowState.handle);
        windowState.handle = nullptr;
    }

    glfwTerminate();
}

