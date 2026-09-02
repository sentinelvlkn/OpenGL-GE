#pragma once

struct GLFWwindow;
struct AppState;
struct GameState;

void updateDebugInput(
    GLFWwindow* window,
    AppState& appState,
    const GameState& gameState
);

