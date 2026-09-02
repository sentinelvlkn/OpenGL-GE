#pragma once

#include <string>
#include <glm/glm.hpp>

struct FadeState;
struct RoomTransitionState;
struct Room;
struct Player;
class Camera;
class Model;
struct GameState;

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

void beginRoomTransition(
							GameState& gameState,
							const std::string& targetRoomJson,
							const glm::vec3& targetSpawn
);

void updateRoomTransition(
							RoomTransitionState& transitionState,
							FadeState& fadeState,
							Room& currentRoom,
							Model*& currentRoomModel,
							Player& player,
							Camera& camera
);

void updateRoomTransition(GameState &gameState);