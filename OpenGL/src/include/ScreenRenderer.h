#pragma once

struct AppState;
struct GameState;
struct FramebufferState;
struct ShaderState;
struct QuadMeshState;

void renderScreen(
							const QuadMeshState& quadMeshState, 
							ShaderState& shaders,
							const FramebufferState& fboState, 
							const AppState& appState,
							const GameState& gameState
);

