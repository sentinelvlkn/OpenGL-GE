#include "include/CameraSystem.h"

void updateRoomCamera(Room &room, const Player &player, Camera &camera) {
	roomCameraSwitch(room, player.position);
	applyRoomCamera(room, camera);
}