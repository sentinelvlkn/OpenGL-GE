#pragma once

struct GLFWwindow;

#include "include/Light.h"

struct LightingDebugState {
	float forcedLightningTimer = 0.0f;

	bool lightningPreview = false;
	bool disableFlicker = false;
	bool printLightInformation = false;
	bool spotAsPoint = false;
};

float calculateFlickerMultiplier(const PointLight& light, float time, int lightIndex, 
	const LightingDebugState& debugState);

void updateLightningDebugInput(GLFWwindow *window, float deltaTime, 
	LightingDebugState& debugState);