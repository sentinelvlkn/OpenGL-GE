#pragma once

struct GLFWwindow;
struct FadeState; 

void processInput(GLFWwindow *window, FadeState &fadeState);
bool keyPressedOnce(GLFWwindow *window, int key);
