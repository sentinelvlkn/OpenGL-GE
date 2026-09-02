#pragma once

struct GLFWwindow;
struct GameState;

void processInput(GLFWwindow *window, GameState &gameState);
bool keyPressedOnce(GLFWwindow *window, int key);
