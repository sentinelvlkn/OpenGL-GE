/*
 * camera.cpp
 *
 *  Created on: Sep 28, 2025
 *      Author: volokin
 *  
 *  completely defaced on 18th Sep 2026 lol 
 */

#include "include/Camera.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(
	const glm::vec3& position,
	const glm::vec3& target,
	float fovDegrees
) : position(position), target(target), up(0.0f, 1.0f, 0.0f), fovDegrees(fovDegrees)
{
}

glm::mat4 Camera::viewMatrix() const {
	return glm::lookAt(position, target, up);
}

void Camera::setView(const glm::vec3& newPosition, const glm::vec3& newTarget) {
	position = newPosition;
	target = newTarget;
}

void Camera::setPosition(const glm::vec3& newPosition) {
	position = newPosition;
}

void Camera::setTarget(const glm::vec3& newTarget) {
	target = newTarget;
}

void Camera::setFov(float newFovDegrees) {
	fovDegrees = std::clamp(newFovDegrees, 1.0f, 120.0f);
}


