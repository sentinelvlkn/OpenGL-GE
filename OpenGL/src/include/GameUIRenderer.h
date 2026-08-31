#pragma once

#include <string>

#include "Shader.h"
#include "TextRenderer.h"
#include "UiRenderer.h"
#include "InteractionSystem.h"

void drawInteractionPrompt(
                            const std::string& promptText,
                            bool shouldDraw,
                            UiRenderer& uiRenderer,
                            Shader& uiRectangleShader,
                            TextRenderer& textRenderer,
                            Shader& textShader,
                            int screenWidth,
                            int screenHeight,
                            float promptScale
);

void drawNoteOverlay(
                            const InteractionRuntimeState& interactionState,
                            Shader& uiShader,
                            unsigned int quadVAO
);