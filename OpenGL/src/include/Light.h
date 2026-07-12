#pragma once

#include <glm/glm.hpp>

struct PointLight {

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 color = glm::vec3(1.0f, 0.8f, 0.5f);

	float intensity = 3.0f;
	float radius = 8.0f;

	bool flicker = false;
};
