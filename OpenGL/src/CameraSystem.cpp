#include "include/CameraSystem.h"

#include "include/GameState.h"
#include "include/Room.h"
#include "include/Player.h"
#include "include/Camera.h"

void updateRoomCamera(GameState &gameState) {
	roomCameraSwitch(gameState.room, gameState.player.position);
	applyRoomCamera(gameState.room, gameState.camera);
}