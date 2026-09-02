#pragma once

class Shader;
class Model;

struct AppState;
struct GameState;

void renderSceneToFBO(
							unsigned int framebuffer,
							Shader& shader,
							Model& playerModel,
							const AppState& appState,
							GameState& gameState
);