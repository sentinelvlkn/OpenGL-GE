#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Room.h"
#include "Player.h"
#include "Camera.h"
#include "Model.h"
#include "FadeSystem.h"

struct RoomTransitionState {
	bool transitioning = false;

	std::string pendingRoomJson;
	glm::vec3 pendingSpawn = glm::vec3(0.0f);
};

void beginRoomTransition(
							RoomTransitionState& transitionState,
							const std::string& targetRoomJson,
							const glm::vec3& targetSpawn,
							FadeState& fadestate
);

void updateRoomTransition(
							RoomTransitionState& transitionState,
							FadeState& fadeState,
							Room& currentRoom,
							Model*& currentRoomModel,
							Player& player,
							Camera& camera
);