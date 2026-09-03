#pragma once

#include <string>

class TextRenderer;
class UiRenderer;

struct AppState;
struct GameState;
struct ShaderState;
struct UiState;

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
                            unsigned int quadVAO
);

