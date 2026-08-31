#pragma once

#include <string>

struct GLFWwindow;

#include "Room.h"
#include "Player.h"
#include "Camera.h"
#include "FadeSystem.h"
#include "InteractionSystem.h"
#include "RoomTransitionSystem.h"

std::string updateGameplay(
                            GLFWwindow* window,
                            Room& room,
                            Player& player,
                            const Camera& camera,
                            RoomTransitionState& roomTransitionState,
                            FadeState& fadeState,
                            InteractionRuntimeState& interactionState,
                            float deltaTime
);