#include "include/FadeSystem.h"

#include <glm/common.hpp>

void updateFade(FadeState &fadeState, float deltaTime) {
	
	if (fadeState.fadeIn) {
		fadeState.value += deltaTime;

		if (fadeState.value >= 1.0f) {
			fadeState.value = 1.0f;
			fadeState.fadeIn = false;
		}

	} else if (fadeState.fadeOut) {
		fadeState.value -= deltaTime;

		if (fadeState.value <= 0.0f) {
			fadeState.value = 0.0f;
			fadeState.fadeOut = false;
		}
	}

	fadeState.value = glm::clamp(fadeState.value, 0.0f, 1.0f);
}

void beginFadeToBlack(FadeState &fadeState) {
	fadeState.value = 0.0f;
	fadeState.fadeOut = false;
	fadeState.fadeIn = true;
}

void beginFadeFromBlack(FadeState &fadeState) {
	fadeState.value = 1.0f;
	fadeState.fadeIn = false;
	fadeState.fadeOut = true;
}

bool isFullyBlack(const FadeState &fadeState) {
	return !fadeState.fadeIn && fadeState.value >= 1.0f;
}

bool isFullyVisible(const FadeState &fadeState) {
	return !fadeState.fadeOut && fadeState.value <= 0.0f;
}