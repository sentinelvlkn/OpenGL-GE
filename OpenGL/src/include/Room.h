#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Camera.h"
#include "AABB.h"
#include "Interaction.h"
#include "Light.h"
#include "Prop.h"
#include "thirdparty/json.hpp"
#include <iostream>

using json = nlohmann::json;

struct RoomCamera {
	glm::vec3 position;
	glm::vec3 target;
	float fov = 45.0f;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};

struct DoorTrigger{
	AABB aabb;
	std::string targetRoomJson;
	glm::vec3 targetSpawn;
};

struct Room {
	std::string name;
	std::string modelPath;
	
	std::vector<RoomCamera> cameras;
	int activeCamera = 0;

	glm::vec3 playerSpawn;
	
	glm::vec3 ambientColor = glm::vec3(0.12f, 0.12f, 0.14f);

	std::vector<AABB> colliders;
	std::vector<DoorTrigger> doors;
	std::vector<Interaction> interactions;
	std::vector<PointLight> lights;
	std::vector<Prop> props;
};



//checks within which RoomCamera's reach the player currently is and sets the active Camera accordingly
inline void roomCameraSwitch(Room &room, const glm::vec3 &playerPos) {
//	std::cout << "player y = " << playerPos.y << "\n";

	if (room.cameras.empty())
		return;

	for (int i = 0; i < room.cameras.size(); ++i) {
		const auto &cam = room.cameras[i];
		if (pointInsideAabb(playerPos, cam.aabbMin, cam.aabbMax)) {
			room.activeCamera = i;
			return;
		}
	}
}

inline void applyRoomCamera(const Room& room, Camera& camera){
	const RoomCamera& shot = room.cameras[room.activeCamera];
	camera.setView(shot.position, shot.target);
	camera.fovDegrees = shot.fov;
}

glm::vec3 convertBlenderCoords(const glm::vec3 &b);

static void convertBlenderAABB(const glm::vec3 &minB, const glm::vec3 &maxB,
							   glm::vec3 &outMin, glm::vec3 &outMax); 

static glm::vec3 readVec3(const json &arr, const char *fieldName);

Room loadRoomFromJson(const std::string &jsonPath);
