#pragma once

struct FrameTiming {
	float currentTime = 0.0f;
	float lastFrameTime = 0.0f;
	float deltaTime = 0.0f;

	float maxDeltaTime = 1.0f / 30.0f;
};

struct AppState {
	int framebufferWidth = 800;
	int framebufferHeight = 600;
	bool framebufferResized = false;

	bool debugTerminalOpen = false;

	float lastMouseX = 400.0f;
	float lastMouseY = 300.0f;
	bool firstMouse = true;

	
	FrameTiming timing;
};