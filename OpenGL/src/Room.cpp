#include "Room.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
//using json = nlohmann::json;

static glm::vec3 readVec3(const json &arr, const char *fieldName) {
	if (!arr.is_array() || arr.size() != 3)
		throw std::runtime_error(std::string("Expected ") + fieldName + " to be an array of 3 numbers");

	return glm::vec3(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>());
}

Room loadRoomFromJson(const std::string &jsonPath) {
	std::cout << "Trying to open room json: " << std::filesystem::absolute(jsonPath) << "\n";
	std::ifstream f(jsonPath);
	if (!f.is_open()) {
		std::cerr << "Failed to open json: " << jsonPath << "\n";
		std::cerr << "Absolute: " << std::filesystem::absolute(jsonPath) << "\n";
		throw std::runtime_error("Failed to open json " + jsonPath);
	}
		

	json j;
	f >> j;

	Room room;
	room.activeCamera = 0;
	room.name = j.value("name", "UnnamedRoom");
	room.modelPath = j.value("model", "");

	if (j.contains("playerSpawn"))
		room.playerSpawn = convertBlenderCoords(readVec3(j.at("playerSpawn"), "playerSpawn"));

	if (!j.contains("cameras") || !j.at("cameras").is_array())
		throw std::runtime_error("Room json missing cameras' array: " + jsonPath);
	
	for (const auto &jc : j.at("cameras")) {
		RoomCamera cam;

		cam.position = convertBlenderCoords(readVec3(jc.at("position"), "position"));
		cam.target = convertBlenderCoords(readVec3(jc.at("target"), "target"));

		glm::vec3 minB = readVec3(jc.at("zoneMin"), "zoneMin");
		glm::vec3 maxB = readVec3(jc.at("zoneMax"), "zoneMax");
		convertBlenderAABB(minB, maxB, cam.aabbMin, cam.aabbMax);
        
        std::cout << "Converted zone (Engine): min("
          << cam.aabbMin.x << "," << cam.aabbMin.y << "," << cam.aabbMin.z
          << ") max("
          << cam.aabbMax.x << "," << cam.aabbMax.y << "," << cam.aabbMax.z
          << ")\n";


		cam.fov = jc.value("fov", 45.0f);
        
        std::cout << "JSON zoneMin (Blender): "
          << minB.x << "," << minB.y << "," << minB.z
          << "  zoneMax: "
          << maxB.x << "," << maxB.y << "," << maxB.z
          << "\n";


		room.cameras.push_back(cam);
	}
	
	if (j.contains("colliders")) {
		if(!j.at("colliders").is_array())
			throw std::runtime_error("colliders is not an array: " + jsonPath);

		for(const auto& c : j["colliders"]) {
			glm::vec3 minB = readVec3(c["min"], "min");
			glm::vec3 maxB = readVec3(c["max"], "max");
		
			glm::vec3 minW, maxW;
			convertBlenderAABB(minB, maxB, minW, maxW);

			glm::vec3 minE = glm::min(minW, maxW);
			glm::vec3 maxE = glm::max(minW, maxW);

			room.colliders.push_back({minE, maxE});

			

			}
	}

	if (j.contains("doors")) {
	    if (!j.at("doors").is_array())
        	throw std::runtime_error("doors is not an array: " + jsonPath);

	    for (const auto& d : j.at("doors")) {
        	DoorTrigger door;

	        glm::vec3 minB = readVec3(d.at("min"), "min");
        	glm::vec3 maxB = readVec3(d.at("max"), "max");

	        glm::vec3 minW, maxW;
        	convertBlenderAABB(minB, maxB, minW, maxW);
	        door.aabb.min = glm::min(minW, maxW);
        	door.aabb.max = glm::max(minW, maxW);

	        door.targetRoomJson = d.value("targetRoom", "");
        	if (door.targetRoomJson.empty())
	            throw std::runtime_error("door missing targetRoom: " + jsonPath);

               if (d.contains("targetSpawn"))
		    door.targetSpawn = convertBlenderCoords(readVec3(d.at("targetSpawn"), "targetSpawn"));
		else
        	    door.targetSpawn = glm::vec3(0.0f);

	        room.doors.push_back(door);
	}
	}
	

	if (room.cameras.empty())
		throw std::runtime_error("Room has 0 cameras: " + jsonPath);


	return room;
}

// blender uses z-up & y-forward, while this engine uses -z-forward & y-up
glm::vec3 convertBlenderCoords(const glm::vec3 &b) {
	return glm::vec3(b.x, b.z, -b.y);
}

static void convertBlenderAABB(const glm::vec3 &minB, const glm::vec3 &maxB,
							   glm::vec3 &outMin, glm::vec3 &outMax) {

	glm::vec3 centerB = (minB + maxB) * 0.5f;
	glm::vec3 extentB = (maxB - minB) * 0.5f;

	glm::vec3 centerE = convertBlenderCoords(centerB);

	glm::vec3 extentE(extentB.x, extentB.z, extentB.y);

	outMin = centerE - extentE;
	outMax = centerE + extentE;
}
