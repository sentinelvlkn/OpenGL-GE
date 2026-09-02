#include "include/DebugInputSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <sstream>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/Input.h"
#include "include/DebugLog.h"

void updateDebugInput(
    GLFWwindow* window,
    AppState& appState,
    const GameState& gameState) {

	if (keyPressedOnce(window, GLFW_KEY_F1)) {
		appState.debugTerminalOpen = !appState.debugTerminalOpen;
	}
	if (keyPressedOnce(window, GLFW_KEY_F2)) {
		std::ostringstream ss;

		ss << "Room: " << gameState.room.name << " | Player: " << gameState.player.position.x
		   << ", " << gameState.player.position.y << ", " << gameState.player.position.z
		   << " | FB: " << appState.framebufferWidth << "x"
		   << appState.framebufferHeight << " | Camera: " << gameState.room.activeCamera
		   << " | Props: " << gameState.room.props.size()
		   << " | Lights: " << gameState.room.lights.size();

		DebugLog::log(ss.str());
	}
}