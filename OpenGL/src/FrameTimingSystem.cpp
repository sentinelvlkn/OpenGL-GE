#include "include/FrameTimingSystem.h"

#include <GLFW/glfw3.h>

#include "include/AppState.h"

void initializeFrameTiming(AppState& appState) {

	float now = static_cast<float>(glfwGetTime());

	appState.timing.currentTime = now;
	
	appState.timing.lastFrameTime = now;

	appState.timing.deltaTime = 0.0f;
}

void updateFrameTiming(AppState& appState) {
	appState.timing.currentTime = static_cast<float>(glfwGetTime());

	appState.timing.deltaTime = appState.timing.currentTime - appState.timing.lastFrameTime;

	appState.timing.lastFrameTime = appState.timing.currentTime;

	if (appState.timing.deltaTime < 0.0f)
		appState.timing.deltaTime = 0.0f;

	if (appState.timing.deltaTime > appState.timing.maxDeltaTime)
		appState.timing.deltaTime = appState.timing.maxDeltaTime;

}