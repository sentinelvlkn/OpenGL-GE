#pragma once

struct AppState;
struct GameState;

struct AppContext {
	AppState* appState = nullptr;
	GameState* gameState = nullptr;
};