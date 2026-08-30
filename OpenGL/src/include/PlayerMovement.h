#pragma once

struct GLFWwindow;

#include "Camera.h"
#include "Player.h"
#include "Room.h"

void updatePlayerMovement(GLFWwindow *window, Player &player,
						  const Camera &camera, const Room &room,
						  float deltaTime);