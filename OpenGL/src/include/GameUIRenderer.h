#pragma once

#include <string>

struct AppState;
struct GameState;
struct ShaderState;
struct UiState;
struct QuadMeshState;

void drawInteractionPrompt(
                            const std::string& promptText,
                            UiState& uiState,
                            ShaderState& shaders,
                            const AppState& appState,
                            const GameState& gameState
);

void drawNoteOverlay(
                            const GameState& gameState,
                            ShaderState& shaders,
                            const QuadMeshState& quadMeshState
);

