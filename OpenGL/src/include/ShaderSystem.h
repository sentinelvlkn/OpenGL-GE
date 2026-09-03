#pragma once

#include "Shader.h"

struct ShaderState {
	Shader sceneShader;
	Shader screenShader;
	Shader uiShader;
	Shader textShader;
	Shader uiRectangleShader;

	ShaderState();
};

void initializeShaders(ShaderState &shaders);