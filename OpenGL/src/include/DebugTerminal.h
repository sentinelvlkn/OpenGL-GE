#pragma once

struct UiRenderer;
struct TextRenderer;
struct AppState;
struct ShaderState;

void drawDebugTerminal(	UiRenderer &uiRenderer, 
						TextRenderer &textRenderer, 
						ShaderState& shaders,
						const AppState &appState
);