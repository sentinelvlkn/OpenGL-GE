#include "include/CameraSystem.h"

#include "include/GameState.h"
#include "include/Room.h"

void updateRoomCamera(GameState &gameState) {
	
	Room& room = gameState.room;

	if (room.cameras.empty())
		return;

	roomCameraSwitch(room, gameState.player.position);

	const RoomCamera& shot = room.cameras[room.activeCamera];

	glm::vec3 target = shot.target;

	if (shot.targetMode == CameraTargetMode::FollowPlayer)
		target = gameState.player.position + shot.targetOffset;

	gameState.camera.setView(shot.position, target);

	gameState.camera.setFov(shot.fov);

}

