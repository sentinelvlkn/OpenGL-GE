#pragma once

struct GLFWwindow;

struct AppState;
struct GameState;
struct AppContext;


struct WindowState {
	GLFWwindow *handle = nullptr;
};

bool initializeWindow(
                        WindowState& windowState,
                        AppState& appState,
                        GameState& gameState,
                        AppContext& appContext,
                        int width,
                        int height,
                        const char* title,
                        bool enableMouseLook = false
);

bool shouldWindowClose(const WindowState &windowState);

void swapWindowBuffers(const WindowState &windowState);

void pollWindowEvents();

void shutdownWindow(WindowState &windowState);