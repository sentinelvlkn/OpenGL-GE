#pragma once

#include <glm/glm.hpp>
#include <string>

struct PointLight {

	std::string type = "point"; //point or spot

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 target = glm::vec3(0.0f);

	glm::vec3 color = glm::vec3(1.0f, 0.8f, 0.5f);

	float intensity = 3.0f;
	float radius = 8.0f;

	//spot light cone angles in degrees
	float innerCutoff = 12.0f;
	float outerCutoff = 28.0f;

	bool flicker = false;

	std::string flickerType = "none";

	float flickerFrequency = 8.0f;
	float flickerStrength = 0.2f;
	float flickerRandomness = 0.0f;

	float flickerThreshold = 0.85f; //for thunderstorm flashes
};
