#include "include/GameplaySystem.h"

#include <GLFW/glfw3.h>

#include "include/Input.h"
#include "include/PlayerMovement.h"

std::string updateGameplay(	GLFWwindow *window, 
							Room &room, 
							Player &player,
							const Camera &camera,
							RoomTransitionState &roomTransitionState,
							FadeState &fadeState,
							InteractionRuntimeState &interactionState,
							float deltaTime) {

	std::string promptText;

	if (roomTransitionState.transitioning)
		return promptText;

	bool pressedE = keyPressedOnce(window, GLFW_KEY_E);

	if (interactionState.messageOpen) {
		if (pressedE) 
			closeActiveInteraction(interactionState);

		return promptText;
	}

	updatePlayerMovement(window, player, camera, room, deltaTime);

	NearbyAction nearbyAction = getNearbyAction(room, player.position);

	promptText = nearbyAction.promptText;

	if (nearbyAction.door && pressedE) 
		beginRoomTransition(roomTransitionState,
							nearbyAction.door->targetRoomJson,
							nearbyAction.door->targetSpawn, 
							fadeState
		);

	else if (nearbyAction.interaction && pressedE)
		runInteraction(*nearbyAction.interaction, interactionState);
	
	return promptText;
}
