#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Camera.h"
#include "thirdparty/json.hpp"
#include <iostream>

using json = nlohmann::json;

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

struct RoomCamera {
	glm::vec3 position;
	glm::vec3 target;
	float fov = 45.0f;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};

struct Room {
	std::string name;
	std::string modelPath;
	
	std::vector<RoomCamera> cameras;
	int activeCamera = 0;

	glm::vec3 playerSpawn;

	std::vector<AABB> colliders;
};

//checks if a point (p) is within boundaries defined with glm::vec3(s) 
inline bool pointInsideAabb(const glm::vec3& p, const glm::vec3& min, const glm::vec3& max) {
	return	p.x >= min.x && p.x <= max.x && 
			p.y >= min.y && p.y <= max.y &&
			p.z >= min.z && p.z <= max.z;
}

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
