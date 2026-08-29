#pragma once

#include <string>
#include <glm/glm.hpp>

struct Prop {
	std::string id;
	std::string modelPath;

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	bool glow = false;
	glm::vec3 glowColor = glm::vec3(1.0f, 0.85f, 0.45f);
	float glowStrength = 1.0f;
	
	bool spin = false;
	glm::vec3 spinAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	float spinSpeed = 0.0f; //degrees per second
};