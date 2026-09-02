#pragma once

#include <string>

class Shader;
class TextRenderer;
class UiRenderer;

struct AppState;
struct GameState;

void drawInteractionPrompt(
                            const std::string& promptText,
                            UiRenderer& uiRenderer,
                            Shader& uiRectangleShader,
                            TextRenderer& textRenderer,
                            Shader& textShader,
                            const AppState& appState,
                            const GameState& gameState,
                            float promptScale
);

void drawNoteOverlay(
                            const GameState& gameState,
                            Shader& uiShader,
                            unsigned int quadVAO
);

