#include "include/GameplaySystem.h"

#include <GLFW/glfw3.h>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/Input.h"
#include "include/PlayerMovement.h"
#include "include/InteractionSystem.h"
#include "include/RoomTransitionSystem.h"

std::string updateGameplay(
                            GLFWwindow* window,
                            GameState& gameState,
                            float deltaTime
) {
	std::string promptText;

	if (gameState.roomTransition.transitioning)
		return promptText;

	bool pressedE = keyPressedOnce(window, GLFW_KEY_E);

	if (gameState.interaction.messageOpen) {
		if (pressedE)
			closeActiveInteraction(gameState.interaction);

		return promptText;
	}

	updatePlayerMovement(window, gameState.player, gameState.camera, gameState.room, deltaTime);

	NearbyAction nearbyAction = getNearbyAction(gameState.room, gameState.player.position);

	promptText = nearbyAction.promptText;

	if (nearbyAction.door && pressedE)
		beginRoomTransition(
			gameState,
			nearbyAction.door->targetRoomJson,
			nearbyAction.door->targetSpawn
		);

	else if (nearbyAction.interaction && pressedE)
		runInteraction(*nearbyAction.interaction, gameState.interaction);

	return promptText;
}

std::string updateGameplay(
							GLFWwindow* window,
							const AppState& appState,
							GameState& gameState
) {
    return updateGameplay(window, gameState, appState.timing.deltaTime);
}