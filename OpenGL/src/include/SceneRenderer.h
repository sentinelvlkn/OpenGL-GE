#pragma once

struct AppState;
struct GameState;
struct FramebufferState;
struct ShaderState;
struct ModelState;

void renderSceneToFBO(
							const FramebufferState& fboState,
							ShaderState& shaders,
							ModelState& models,
							const AppState& appState,
							GameState& gameState
);