#pragma once

struct GLFWwindow;

void processInput(GLFWwindow *window, float &fade, bool &fadeIn, bool &fadeOut);
bool keyPressedOnce(GLFWwindow *window, int key);
