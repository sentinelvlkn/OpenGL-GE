#include "Room.h"
#include "Interaction.h"
#include "Light.h"
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

	if (j.contains("interactions")) {
		if (!j.at("interactions").is_array())
			throw std::runtime_error ("interactions is not an array: " + jsonPath);
			
		for (const auto& ji : j.at("interactions")) {
			Interaction interaction;

			interaction.id = ji.value("id", "");
			interaction.type = interactionTypeFromString(ji.value("type", "unknown"));

			interaction.message = ji.value("message", "");
			interaction.assetPath = ji.value("asset", "");
			interaction.targetId = ji.value("targetId", "");

			interaction.repeatable = ji.value("repeatable", true);
			interaction.used = false;

			glm::vec3 minB = readVec3(ji.at("min"), "interaction min");
			glm::vec3 maxB = readVec3(ji.at("max"), "interaction max");

			glm::vec3 minW, maxW;
			convertBlenderAABB(minB, maxB, minW, maxW);

			interaction.trigger.min = glm::min(minW, maxW);
			interaction.trigger.max = glm::max(minW, maxW);

			room.interactions.push_back(interaction);
		}
	}

	if (j.contains("lights")) {
		if (!j.at("lights").is_array())
	        throw std::runtime_error("lights is not an array: " + jsonPath);

		for (const auto& jl : j.at("lights")) {
		        PointLight light;

				light.type = jl.value("type", "point");

		        glm::vec3 posB = readVec3(jl.at("position"), "light position");
		        light.position = convertBlenderCoords(posB);

				if (jl.contains("target")) {
					glm::vec3 targetB = readVec3(jl.at("target"), "light target");
					light.target = convertBlenderCoords(targetB);
				} 
				else {
					light.target = light.position + glm::vec3(0.0f, 0.0f, -1.0f);
				}

				if (jl.contains("color")) {
					light.color = readVec3(jl.at("color"), "light color");
        		}

		        light.intensity = jl.value("intensity", 3.0f);
		        light.radius = jl.value("radius", 8.0f);

				light.innerCutoff = jl.value("innerCutoff", 12.0f);
				light.outerCutoff = jl.value("outerCutoff", 28.0f);
		        
				if (jl.contains("flicker")) {
					const auto& jf = jl.at("flicker");

					if (jf.is_boolean()) {
						light.flicker = jf.get<bool>();
						light.flickerType = light.flicker ? "sine" : "none";
					}
					else if (jf.is_object()) {
						light.flicker = jf.value("enabled", false);
						light.flickerType = jf.value("type", "sine");

						light.flickerFrequency = jf.value("frequency", 8.0f);
						light.flickerStrength = jf.value("strength", 0.2f);
						light.flickerRandomness = jf.value("randomness", 0.0f);
						light.flickerThreshold = jf.value("threshold", 0.85f);
					}
				}

				room.lights.push_back(light);
    		}
	}	

	if (room.cameras.empty())
		throw std::runtime_error("Room has 0 cameras: " + jsonPath);

	if (j.contains("ambientColor")) {
		room.ambientColor = readVec3(j.at("ambientColor"), "ambientColor");

		std::cout << "Loaded ambientColor for " << room.name << ": "
              	        << room.ambientColor.x << ", "
			<< room.ambientColor.y << ", "
			<< room.ambientColor.z << "\n";	
	}
	else {
		room.ambientColor = glm::vec3(0.12f, 0.12f, 0.14f);
		std::cout << "No ambientColor in " << room.name
		              << ", using default.\n";
	}


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
