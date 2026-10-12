#include "include/RoomTransitionSystem.h"

#include <exception>
#include <string>

#include "include/AssetCache.h"
#include "include/CameraSystem.h"
#include "include/FadeSystem.h"
#include "include/DebugLog.h"
#include "include/FileSystem.h"
#include "include/GameState.h"
#include "include/Room.h"

void beginRoomTransition(
							GameState& gameState,
							const std::string& targetRoomJson,
							const glm::vec3& targetSpawn
) {
	gameState.roomTransition.transitioning = true;
	gameState.roomTransition.pendingRoomJson = targetRoomJson;
	gameState.roomTransition.pendingSpawn = targetSpawn;
	
	beginFadeToBlack(gameState.fade);
}


void updateRoomTransition(GameState& gameState)
{
	if (!gameState.roomTransition.transitioning)
		return;

	if (!isFullyBlack(gameState.fade))
		return;
	
	try {
		std::string roomPath = FileSystem::pathFromRoot(
			gameState.roomTransition.pendingRoomJson
		);

		gameState.room = loadRoomFromJson(roomPath);

		gameState.roomModel = &getRoomModel(gameState.room);

		gameState.player.position = gameState.roomTransition.pendingSpawn;

		updateRoomCamera(gameState);

		DebugLog::log(
			"[ROOM] Loaded: " + gameState.room.name
		);

		beginFadeFromBlack(gameState.fade);

		gameState.roomTransition.transitioning = false;
		gameState.roomTransition.pendingRoomJson.clear();

	} catch(const std::exception& e) {

		DebugLog::log(
			std::string("[ROOM ERROR] Failed to load room: ") + e.what()
		);

		beginFadeFromBlack(gameState.fade);

		gameState.roomTransition.transitioning = false;
		gameState.roomTransition.pendingRoomJson.clear();
	}
}


