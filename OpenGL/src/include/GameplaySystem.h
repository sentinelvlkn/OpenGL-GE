#pragma once

#include <string>

struct GLFWwindow;
struct AppState;
struct GameState;

std::string updateGameplay(
                            GLFWwindow* window,
                            GameState& gameState,
                            float deltaTime
);

std::string updateGameplay(
                            GLFWwindow* window,
                            const AppState& appState,
                            GameState& gameState
);