#include "include/ScreenRenderer.h"
#include "include/FramebufferSystem.h"
#include "include/ShaderSystem.h"

#include <glad/glad.h>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/Shader.h"



void renderScreen(
							unsigned int quadVAO, 
							ShaderState& shaders,
							const FramebufferState& fboState, 
							const AppState& appState,
							const GameState& gameState
) {
	
	Shader &screenShader = shaders.screenShader;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, appState.framebufferWidth, appState.framebufferHeight);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();

	screenShader.setInt("screenTexture", 0);

	screenShader.setFloat("fade", gameState.fade.value);
	screenShader.setFloat("time", appState.timing.currentTime);

	screenShader.setFloat("vignetteStrength", 0.0f);
	screenShader.setFloat("grainStrength", 0.0f);
	screenShader.setFloat("saturation", 1.0f);
	screenShader.setFloat("contrast", 1.0f);
	screenShader.setFloat("brightness", 0.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboState.textureColorbuffer);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
