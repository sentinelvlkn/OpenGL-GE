#include "include/UiRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

UiRenderer::UiRenderer() {
	glGenVertexArrays(1, &rectangleVAO);
	glGenBuffers(1, &rectangleVBO);

	glBindVertexArray(rectangleVAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);

	glBufferData(
					GL_ARRAY_BUFFER, 
					sizeof(float) * 2 * 6,
					nullptr,
					GL_DYNAMIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(	0, 
							2, 
							GL_FLOAT, 
							GL_FALSE, 
							2 * sizeof(float),
							(void *)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

UiRenderer::~UiRenderer() { 
	if (rectangleVBO != 0)
		glDeleteBuffers(1, &rectangleVBO);

	if (rectangleVAO != 0)
		glDeleteVertexArrays(1, &rectangleVAO);
}

void UiRenderer::drawRectangle( Shader &shader, 
								float x, 
								float y, 
								float width,
								float height, 
								const glm::vec3 &color,
								float alpha, 
								int screenWidth, 
								int screenHeight) {
	
	if (width <= 0.0f || height <= 0.0f)
		return;

	float vertices[] = {
	
		x,			y + height, 
		x,			y, 
		x + width,	y,

		x,			y + height, 
		x + width,	y, 
		x + width,	y + height
	
	};

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader.use();

	glm::mat4 projection = glm::ortho(
		0.0f, 
		static_cast<float>(screenWidth),
		0.0f, 
		static_cast<float>(screenHeight)
	);

	shader.setMat4("projection", projection);
	shader.setVec3("rectangleColor", color);
	shader.setFloat("rectangleAlpha", alpha);

	glBindVertexArray(rectangleVAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDisable(GL_BLEND);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}	