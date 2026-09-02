#pragma once

struct AppState {
	int framebufferWidth = 800;
	int framebufferHeight = 600;
	bool framebufferResized = false;

	bool debugTerminalOpen = false;
};