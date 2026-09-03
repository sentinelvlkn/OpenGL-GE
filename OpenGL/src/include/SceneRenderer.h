#pragma once

class Shader;
class Model;

struct AppState;
struct GameState;
struct FramebufferState;

void renderSceneToFBO(
							const FramebufferState& fboState,
							Shader& shader,
							Model& playerModel,
							const AppState& appState,
							GameState& gameState
);