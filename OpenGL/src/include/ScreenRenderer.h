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
							const GameState& gameState
);

