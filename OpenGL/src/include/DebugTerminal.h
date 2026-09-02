#pragma once

struct UiRenderer;
struct Shader;
struct TextRenderer;
struct AppState;

void drawDebugTerminal(	UiRenderer &uiRenderer, Shader &uiRectangleShader,
						TextRenderer &textRenderer, Shader &textShader,
						const AppState &appState);