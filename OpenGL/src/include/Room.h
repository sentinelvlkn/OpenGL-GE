#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Camera.h"
#include "AABB.h"
#include "Interaction.h"
#include "Light.h"
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

	std::vector<AABB> colliders;
	std::vector<DoorTrigger> doors;
	std::vector<Interaction> interactions;
	std::vector<PointLight> lights;
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

//applies the active camera after setting the index in roomCameraSwitch
//inline void applyRoomCamera(const Room& room, Camera& camera) {
//	const RoomCamera &shot = room.cameras[room.activeCamera];

	//camera.setLookAt(shot.position, shot.target); make it snap immediately
	
	//the code down does blending and makes it more fancy lol
//	glm::vec3 targetPos = shot.position;
//	glm::vec3 targetFront = glm::normalize(shot.target - shot.position);

//	 const float blend = 0.08f;

//	camera.Position = glm::mix(camera.Position, targetPos, blend);
//	camera.Front = glm::normalize(glm::mix(camera.Front, targetFront, blend));

//	camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
//	camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

//	camera.Zoom = glm::mix(camera.Zoom, shot.fov, blend);

	/*std::cout << "Cam pos " << shot.position.x << "," << shot.position.y << ","
			  << shot.position.z << " target " << shot.target.x << ","
			  << shot.target.y << "," << shot.target.z << "\n";*/
//}

inline void applyRoomCamera(const Room& room, Camera& camera){
	const RoomCamera& shot = room.cameras[room.activeCamera];
	camera.setLookAt(shot.position, shot.target);
	camera.Zoom = shot.fov;
}

glm::vec3 convertBlenderCoords(const glm::vec3 &b);

static void convertBlenderAABB(const glm::vec3 &minB, const glm::vec3 &maxB,
							   glm::vec3 &outMin, glm::vec3 &outMax); 

static glm::vec3 readVec3(const json &arr, const char *fieldName);

Room loadRoomFromJson(const std::string &jsonPath);
