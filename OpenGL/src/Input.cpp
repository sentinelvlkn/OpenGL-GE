#include "include/Input.h"
#include <GLFW/glfw3.h>

void processInput(GLFWwindow *window, float &fade, bool &fadeIn, bool &fadeOut) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	// fadeIn sets to true
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
		fade = 0.0f;
		fadeIn = true;
	}
	// fadeOut sets to true
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
		fade = 1.0f;
		fadeOut = true;
	}
}

bool keyPressedOnce(GLFWwindow *window, int key) {

	static bool wasDown[GLFW_KEY_LAST + 1] = {false};
	bool down = glfwGetKey(window, key) == GLFW_PRESS;
	bool pressed = down && !wasDown[key];
	wasDown[key] = down;
	return pressed;
}