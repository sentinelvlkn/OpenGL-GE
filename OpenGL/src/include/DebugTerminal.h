#pragma once

struct UiRenderer;
struct TextRenderer;
struct AppState;
struct ShaderState;
struct UiState;

void drawDebugTerminal(	UiState& uiState,
						ShaderState& shaders,
						const AppState &appState
);