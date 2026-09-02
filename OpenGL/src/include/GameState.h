#pragma once

#include <glm/glm.hpp>

#include "Camera.h"
#include "Player.h"
#include "Room.h"

#include "FadeSystem.h"
#include "InteractionSystem.h"
#include "LightingSystem.h"
#include "RoomTransitionSystem.h"

class Model;

struct GameState {
	Room room;
	Model *roomModel = nullptr;

	Player player;
	Camera camera;

	FadeState fade;
	RoomTransitionState roomTransition;
	InteractionRuntimeState interaction;
	LightingDebugState lightingDebug;

	GameState() :
		camera(glm::vec3(0.0f, 0.0f, 5.0f)) {}
};