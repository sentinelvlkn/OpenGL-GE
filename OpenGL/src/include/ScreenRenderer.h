#pragma once

#include "Shader.h"
#include "Model.h"
#include "Player.h"
#include "Room.h"
#include "Camera.h"
#include "LightingSystem.h"

void renderScreen(
							unsigned int quadVAO, 
							Shader &screenShader,
							unsigned int textureColorBuffer, 
							float time,
							float fadeValue, 
							int width, 
							int height
);

void renderSceneToFBO(
							unsigned int framebuffer, 
							Shader& shader,
							Model& roomModel, 
							Model& playerModel,
							const Player& player, 
							const Room& room,
							Camera& camera,
							int fboWidth,
							int fboHeight,
							float time,
							LightingDebugState& lightingDebugState
);