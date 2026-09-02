#include "include/GameUIRenderer.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/InteractionSystem.h"
#include "include/Shader.h"
#include "include/TextRenderer.h"
#include "include/UiRenderer.h"

void drawInteractionPrompt(
							const std::string& promptText,
							UiRenderer& uiRenderer,
							Shader& uiRectangleShader,
							TextRenderer& textRenderer,
							Shader& textShader,
							const AppState& appState,
							const GameState& gameState,
							float promptScale
) {
    
	if (promptText.empty())
		return;
	
    if (gameState.interaction.messageOpen)
		return;

	if (gameState.interaction.noteOpen)
		return;

	if (gameState.roomTransition.transitioning)
		return;

	float screenWidth = static_cast<float>(appState.framebufferWidth);
	float screenHeight = static_cast<float>(appState.framebufferHeight);

	float promptWidth = textRenderer.measureTextWidth(promptText, promptScale);
    float promptX = (screenWidth * 0.5f) - (promptWidth * 0.5f);

	float promptY = 80.0f;

	float paddingX = 18.0f;
	float rectangleX = promptX - paddingX;
	float rectangleY = promptY - 12.0f;
	float rectangleW = promptWidth + paddingX * 2.0f;
	float rectangleH = 36.0f;

	uiRenderer.drawRectangle(
		uiRectangleShader,
		rectangleX,
		rectangleY,
		rectangleW,
		rectangleH,
		glm::vec3(0.0f, 0.0f, 0.0f),
		0.55f,
		screenWidth,
		screenHeight
	);

	textRenderer.renderText(
		textShader,
		promptText,
		promptX,
		80.0f,
		promptScale,
		glm::vec3(1.0f, 0.9f, 0.65f),
		screenWidth,
		screenHeight
	);
}

void drawNoteOverlay(
                            const GameState& gameState,
							Shader& uiShader,
							unsigned int quadVAO
) {
	const InteractionRuntimeState &interactionState = gameState.interaction;

	if (!interactionState.noteOpen || interactionState.activeNoteTexture == 0)
		return;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	uiShader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, interactionState.activeNoteTexture);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}