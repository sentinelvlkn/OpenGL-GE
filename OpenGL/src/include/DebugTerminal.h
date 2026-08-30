#pragma once

#include "Shader.h"
#include "TextRenderer.h"
#include "UiRenderer.h"

void drawDebugTerminal(	UiRenderer &uiRenderer, Shader &uiRectangleShader,
						TextRenderer &textRenderer, Shader &textShader,
						int screenWidth, int screenHeight, bool open);