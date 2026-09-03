#pragma once

class Shader;
class Model;

struct AppState;
struct GameState;
struct FramebufferState;
struct ShaderState;

void renderSceneToFBO(
							const FramebufferState& fboState,
							ShaderState& shaders,
							Model& playerModel,
							const AppState& appState,
							GameState& gameState
);