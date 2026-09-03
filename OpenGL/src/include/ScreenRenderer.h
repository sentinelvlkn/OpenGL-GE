#pragma once


class Shader;
class Model;

struct AppState;
struct GameState;
struct FramebufferState;

void renderScreen(
							unsigned int quadVAO, 
							Shader& screenShader,
							const FramebufferState& fboState, 
							const AppState& appState,
							const GameState& gameState
);

