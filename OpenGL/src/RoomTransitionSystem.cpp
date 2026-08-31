#include "include/RoomTransitionSystem.h"

#include <exception>
#include <string>

#include "include/AssetCache.h"
#include "include/DebugLog.h"
#include "include/FileSystem.h"

void beginRoomTransition(	RoomTransitionState &transitionState,
							const std::string &targetRoomJson,
							const glm::vec3 &targetSpawn, 
							FadeState &fadeState) {

	transitionState.transitioning = true;
	transitionState.pendingRoomJson = targetRoomJson;
	transitionState.pendingSpawn = targetSpawn;

	beginFadeToBlack(fadeState);
}

void updateRoomTransition(RoomTransitionState &transitionState,
						  FadeState &fadeState, Room &currentRoom,
						  Model *&currentRoomModel, Player &player,
						  Camera &camera) {

	if (!transitionState.transitioning)
		return;

	if (!isFullyBlack(fadeState))
		return;

	try {
		
		std::string roomPath = FileSystem::pathFromRoot(
			transitionState.pendingRoomJson
		);

		Room loadedRoom = loadRoomFromJson(roomPath.c_str());

		currentRoom = loadedRoom;
		currentRoomModel = &getRoomModel(currentRoom);

		player.position = transitionState.pendingSpawn;

		roomCameraSwitch(currentRoom, player.position);
		applyRoomCamera(currentRoom, camera);

		DebugLog::log("[ROOM] Loaded: " + currentRoom.name);

		beginFadeFromBlack(fadeState);

		transitionState.transitioning = false;
		transitionState.pendingRoomJson.clear();
	} 
	catch (const std::exception &e) {
		DebugLog::log(std::string("[ROOM ERROR] Failed to load room: ") +
					  e.what()
		);
		beginFadeFromBlack(fadeState);

		transitionState.transitioning = false;
		transitionState.pendingRoomJson.clear();
	}
}