#pragma once


class Shader;
class Model;

struct AppState;
struct GameState;

void renderScreen(
							unsigned int quadVAO, 
							Shader& screenShader,
							unsigned int textureColorBuffer, 
							const AppState& appState,
							const GameState& gameState,
							float time
);


void renderSceneToFBO(
							unsigned int framebuffer,
							Shader& shader,
							Model& playerModel,
							const AppState& appState,
							GameState& gameState,
							float time
);