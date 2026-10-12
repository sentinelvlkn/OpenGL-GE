#include "include/ScreenRenderer.h"
#include "include/FramebufferSystem.h"
#include "include/ShaderSystem.h"
#include "include/QuadMeshSystem.h"

#include <glad/glad.h>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/Shader.h"



void renderScreen(
							const QuadMeshState& quadMeshState, 
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

	screenShader.setUniform("screenTexture", 0);

	screenShader.setUniform("fade", gameState.fade.value);
	screenShader.setUniform("time", appState.timing.currentTime);

	screenShader.setUniform("vignetteStrength", 0.0f);
	screenShader.setUniform("grainStrength", 0.0f);
	screenShader.setUniform("saturation", 1.0f);
	screenShader.setUniform("contrast", 1.0f);
	screenShader.setUniform("brightness", 0.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboState.textureColorbuffer);

	glBindVertexArray(quadMeshState.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
