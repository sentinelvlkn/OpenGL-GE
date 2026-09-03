#pragma once

#include <string>

class TextRenderer;
class UiRenderer;

struct AppState;
struct GameState;
struct ShaderState;

void drawInteractionPrompt(
                            const std::string& promptText,
                            UiRenderer& uiRenderer,
                            TextRenderer& textRenderer,
                            ShaderState& shaders,
                            const AppState& appState,
                            const GameState& gameState,
                            float promptScale
);

void drawNoteOverlay(
                            const GameState& gameState,
                            ShaderState& shaders,
                            unsigned int quadVAO
);

