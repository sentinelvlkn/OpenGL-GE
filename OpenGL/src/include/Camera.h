#pragma once

/*
 * camera.h
 *
 *  Created on: Sep 27, 2025
 *      Author: volokin
 */


#include <glm/glm.hpp>

class Camera {
public:
	glm::vec3 position;
	glm::vec3 up;
	glm::vec3 target;

	float fovDegrees;
	
	explicit Camera(
		const glm::vec3& position = glm::vec3(0.0f, 0.0f, 5.0f),
		const glm::vec3& target = glm::vec3(0.0f),
		float fovDegrees = 45.0f
	);

	glm::mat4 viewMatrix() const;

	void setView(
		const glm::vec3& newPosition,
		const glm::vec3& newTarget
	);

	void setPosition(const glm::vec3& newPosition);

	void setTarget(const glm::vec3& newTarget);

	void setFov(float newFovDegrees);

};

