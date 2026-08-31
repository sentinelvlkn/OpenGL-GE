#include "include/GameUIRenderer.h"

void drawInteractionPrompt(
                            const std::string& promptText,
                            bool shouldDraw,
                            UiRenderer& uiRenderer,
                            Shader& uiRectangleShader,
                            TextRenderer& textRenderer,
                            Shader& textShader,
                            int screenWidth,
                            int screenHeight,
                            float promptScale) {
	
    if (!shouldDraw)
		return;

    if (promptText.empty())
		return;

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
                            const InteractionRuntimeState& interactionState,
                            Shader& uiShader,
                            unsigned int quadVAO) {
	
	if (!interactionState.noteOpen)
		return;

	if (interactionState.activeNoteTexture == 0)
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