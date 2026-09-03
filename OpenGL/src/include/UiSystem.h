#pragma once

#include "TextRenderer.h"
#include "UiRenderer.h"

struct UiState {
	TextRenderer textRenderer;
	UiRenderer uiRenderer;

	float promptScale = 0.75f;
};

bool initializeUi(UiState& uiState);