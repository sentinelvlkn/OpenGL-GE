#include "include/PlayerMovement.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include "include/AABB.h"
#include "include/DebugLog.h"

static bool circleAABBIntersectXZ(const glm::vec3 &pos, float radius,
								  const AABB &box) {
	float closestX = glm::clamp(pos.x, box.min.x, box.max.x);
	float closestZ = glm::clamp(pos.z, box.min.z, box.max.z);

	float dx = pos.x - closestX;
	float dz = pos.z - closestZ;

	return (dx * dx + dz * dz) < (radius * radius);
}

static glm::vec3 getMoveDirInput(GLFWwindow *window,
								 const glm::vec3 &camForward,
								 const glm::vec3 &camRight) {
	glm::vec3 moveDir(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += camForward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= camForward;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= camRight;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += camRight;

	if (glm::length(moveDir) > 0.0f)
		moveDir = glm::normalize(moveDir);

	return moveDir;
}

void updatePlayerMovement(GLFWwindow *window, Player &player,
								 const Camera &camera, const Room &room,
								 float deltaTime) {
	// clamp dt to reduce tunneling on frame spikes
	deltaTime = glm::min(deltaTime, 1.0f / 30.0f);

	// camera-relative basis (flattened onto XZ)
	glm::vec3 camForward = camera.target - camera.position;
	camForward.y = 0.0f;

	if (glm::length(camForward) > 0.0001f)
		camForward = glm::normalize(camForward);
	else
		camForward = glm::vec3(0.0f, 0.0f, -1.0f);

	glm::vec3 camRight = glm::cross(camForward, glm::vec3(0.0f, 1.0f, 0.0f));

	if (glm::length(camRight) > 0.0001f)
		camRight = glm::normalize(camRight);
	else
		camRight = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 moveDir = getMoveDirInput(window, camForward, camRight);

	if (glm::length(moveDir) < 0.0001f)
		return;

	const float playerRadius = 0.40f;

	// substeps
	const int steps = 4;
	const float dt = deltaTime / static_cast<float>(steps);

	static int lastHit = -1;

	for (int s = 0; s < steps; ++s) {
		glm::vec3 nextPos = player.position + moveDir * player.speed * dt;

		bool blocked = false;
		int hitIndex = -1;

		for (int i = 0; i < (int)room.colliders.size(); ++i) {
			if (circleAABBIntersectXZ(nextPos, playerRadius, room.colliders[i])) {
				blocked = true;
				hitIndex = i;
				break;
			}
		}

		if (blocked) {
			if (hitIndex != lastHit) {
				DebugLog::log("[COLLISION] Blocked by collider #" +
							  std::to_string(hitIndex));
				lastHit = hitIndex;
			}
			return; // stop moving further this frame
		}

		// not blocked -> commit this substep
		player.position = nextPos;
		lastHit = -1;
	}
}
