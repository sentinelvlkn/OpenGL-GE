#pragma once

struct FadeState {
	float value = 1.0f;

	bool fadeIn = false; // visible to black

	bool fadeOut = true; // black to visible
};

void updateFade(FadeState &fadeState, float deltaTime);

void beginFadeToBlack(FadeState& fadeState);

void beginFadeFromBlack(FadeState& fadeState);

bool isFullyBlack(const FadeState& fadeState);

bool isFullyVisible(const FadeState& fadeState);