#include "include/GameBootstrap.h"

#include "include/GameState.h"
#include "include/AssetCache.h"
#include "include/CameraSystem.h"
#include "include/DebugLog.h"
#include "include/FileSystem.h"
#include "include/Room.h"

void initializeGameState(GameState& gameState, const char* initialRoomJson) {
	
	gameState.room = loadRoomFromJson(
		FileSystem::pathFromRoot(initialRoomJson).c_str()
	);

	gameState.roomModel = &getRoomModel(gameState.room);

	gameState.player.position = gameState.room.playerSpawn;

	updateRoomCamera(gameState);

	DebugLog::log("[BOOT] Loaded initial room: " + gameState.room.name);

}