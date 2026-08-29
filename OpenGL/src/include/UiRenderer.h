#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.h"

class UiRenderer {

public:
	UiRenderer();
	~UiRenderer();

	void drawRectangle(	Shader &shader, 
						float x, 
						float y, 
						float width,
						float height, 
						const glm::vec3 &color,	
						float alpha,
						int screenWidth, 
						int screenHeight
	);

private:
	unsigned int rectangleVAO = 0;
	unsigned int rectangleVBO = 0;

};