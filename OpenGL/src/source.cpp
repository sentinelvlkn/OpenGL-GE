#include "include/AppContext.h"
#include "include/AppState.h"
#include "include/GameState.h"

#include "include/WindowSystem.h"
#include <include/Input.h>
#include "include/DebugLog.h"
#include "include/DebugTerminal.h"
#include "include/FramebufferSystem.h"
#include "include/ScreenRenderer.h"
#include "include/SceneRenderer.h"
#include "include/CameraSystem.h"
#include "include/GameplaySystem.h"
#include "include/GameUIRenderer.h"
#include "include/DebugInputSystem.h"
#include "include/FrameTimingSystem.h"
#include "include/GameBootstrap.h"
#include "include/ShaderSystem.h"
#include "include/UiSystem.h"
#include "include/QuadMeshSystem.h"
#include "include/ModelSystem.h"

constexpr int INITIAL_WIDTH = 800;
constexpr int INITIAL_HEIGHT = 600;

int main()
{
	AppState appState;
	GameState gameState;
	AppContext appContext;
	WindowState windowState;

	if (!initializeWindow(windowState, appState, gameState, appContext, INITIAL_WIDTH, INITIAL_HEIGHT, "OPENGL")) {
        return -1;
    }

	ShaderState shaders;
	initializeShaders(shaders);

	initializeGameState(gameState, "src/models/test_room/test_room.json");

	ModelState modelState;
	initializeModels(modelState);

	QuadMeshState quadMeshState;
	initializeScreenQuad(quadMeshState);

	UiState uiState;
	if (!initializeUi(uiState))
		DebugLog::log("[BOOT ERROR] UI initialization failed.");

	FramebufferState fboState;
	if (!initializeFramebuffer(fboState, appState)) {
		destroyScreenQuad(quadMeshState);
		shutdownWindow(windowState);
		return -1;
	}
	
	initializeFrameTiming(appState);

	//main game loop
	while (!shouldWindowClose(windowState)) {

		updateFrameTiming(appState);

		std::string promptText;

		if (appState.framebufferResized) {
			resizeFramebufferAttachments(fboState, appState);
		}

		processInput(windowState.handle, gameState);

		updateLightningDebugInput(windowState.handle, appState, gameState);

		updateDebugInput(windowState.handle, appState, gameState);

		updateFade(gameState, appState);

		updateRoomTransition(gameState);

		promptText = updateGameplay(windowState.handle, appState, gameState);

		if (!gameState.roomTransition.transitioning)
			updateRoomCamera(gameState);

		renderSceneToFBO(fboState, shaders, modelState, appState, gameState);

		renderScreen(quadMeshState, shaders, fboState, appState, gameState);

		drawInteractionPrompt(
			promptText,
			uiState,
			shaders,
			appState,
			gameState
		);

		drawNoteOverlay(gameState, shaders, quadMeshState);

		drawDebugTerminal(uiState, shaders, appState);

		swapWindowBuffers(windowState);
		pollWindowEvents();
	}
	
	destroyFramebuffer(fboState);
	destroyScreenQuad(quadMeshState);
		
	shutdownWindow(windowState);
	return 0;
}	











