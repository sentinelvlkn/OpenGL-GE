#pragma once


class Shader;
class Model;

struct AppState;
struct GameState;
struct FramebufferState;
struct ShaderState;

void renderScreen(
							unsigned int quadVAO, 
							ShaderState& shaders,
							const FramebufferState& fboState, 
							const AppState& appState,
							const GameState& gameState
);

