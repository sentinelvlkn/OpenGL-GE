#include "include/LightingSystem.h"

#include <cmath>
#include <string>

#include <GLFW/glfw3.h>

#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/DebugLog.h"
#include "include/Input.h"

static float hashFloat(float x) {
	return glm::fract(sin(x * 12.9898f) * 43758.5453f);
}

static float noise1D(float x) {
	float i = floor(x);
	float f = glm::fract(x);

	float a = hashFloat(i);
	float b = hashFloat(i + 1.0f);

	float u = f * f * (3.0f - 2.0f * f);

	return glm::mix(a, b, u);
}

float calculateFlickerMultiplier(	const PointLight &light, 
									float time,
									int lightIndex,
									const LightingDebugState &debugState) {
	if (debugState.disableFlicker)
		return 1.0f;

	if (light.flickerType == "lightning") {
		if (debugState.forcedLightningTimer > 0.0f)
			return light.flickerStrength * 4.0f;

		if (debugState.lightningPreview)
			return 1.0f;
	}

	if (!light.flicker)
		return 1.0f;

	float freq = glm::max(light.flickerFrequency, 0.001f);
	float strength = glm::clamp(light.flickerStrength, 0.0f, 1.0f);
	float randomness = glm::clamp(light.flickerRandomness, 0.0f, 1.0f);

	float phase = lightIndex * 17.31f;
	float x = time * freq + phase;

	if (light.flickerType ==
		"fluorescent") { // fast small pulse like worn out halogen tubes
		float pulse = 0.5f + 0.5f * sin(x * 6.28318f);

		float noise = noise1D(x * 2.7f) * 2.0f - 1.0f;

		float variation =
			(pulse - 0.5f) * strength + noise * strength * randomness;

		return glm::clamp(1.0f + variation, 0.15f, 1.35f);
	}

	if (light.flickerType == "tungsten") { // irregular voltage incadescnet
		float slow = sin(x * 6.283f) * 0.5f;

		float n = noise1D(x * 1.35f) * 2.0f - 1.0f;

		float variation = slow * strength * 0.45f + n * strength * randomness;

		return glm::clamp(1.0f + variation, 0.25f, 1.25f);
	}

	if (light.flickerType == "lightning") { // thunderstorm, occasional flash
		float n = noise1D(x);

		if (n < light.flickerThreshold)
			return 0.0f;

		float flashAmount = (n - light.flickerThreshold) /
							glm::max(1.0f - light.flickerThreshold, 0.001f);

		float strobe =
			0.65f + 0.35f * sin(time * 80.0f +
								phase); // rapid pulse after initial lightning
										// to simulate several flashes

		return glm::clamp(flashAmount * strobe * strength * 4.0f, 0.0f, 4.0f);
	}

	// default sine flicker
	float simple = 0.85f + 0.15f * sin(time * freq + phase);
	return glm::mix(1.0f, simple, strength);
}

void updateLightningDebugInput(	GLFWwindow *window, 
								float deltatime,
								GameState& gameState) {

	LightingDebugState& debugState = gameState.lightingDebug;

	if (keyPressedOnce(window, GLFW_KEY_F5)) {
		debugState.forcedLightningTimer = 0.25f;
		DebugLog::log("[LIGHT DEBUG] Forced lightning flash triggered");
	}

	if (keyPressedOnce(window, GLFW_KEY_F6)) {
		debugState.lightningPreview = !debugState.lightningPreview;
		DebugLog::log(std::string("[LIGHT DEBUG] Lightning preview ") +
					  (debugState.lightningPreview ? "ON" : "OFF"));
	}

	if (keyPressedOnce(window, GLFW_KEY_F7)) {
		debugState.disableFlicker = !debugState.disableFlicker;
		DebugLog::log(std::string("[LIGHT DEBUG] Disable flicker: ") +
					  (debugState.disableFlicker ? "ON" : "OFF"));
	}

	if (keyPressedOnce(window, GLFW_KEY_F8)) {
		debugState.printLightInformation = !debugState.printLightInformation;
		DebugLog::log(std::string("[LIGHT DEBUG] Print light information: ") +
					  (debugState.printLightInformation ? "ON" : "OFF"));
	}

	if (keyPressedOnce(window, GLFW_KEY_F9)) {
		debugState.spotAsPoint = !debugState.spotAsPoint;
		DebugLog::log(std::string("[LIGHT DEBUG] Spot as point: ") +
					  (debugState.spotAsPoint ? "ON" : "OFF"));
	}

	if (debugState.forcedLightningTimer > 0.0f) {
		debugState.forcedLightningTimer -= deltatime;
		if (debugState.forcedLightningTimer <= 0.0f) {
			debugState.forcedLightningTimer = 0.0f;
			DebugLog::log("[LIGHT DEBUG] Forced lightning flash ended");
		}
	}
}

void updateLightningDebugInput(GLFWwindow *window, const AppState &appState, GameState &gameState) {
	updateLightningDebugInput(window, appState.timing.deltaTime, gameState);
}