#include "include/FramebufferSystem.h"
#include "include/AppState.h"

#include <glad/glad.h>

#include <string>

#include "include/DebugLog.h"

bool initializeFramebuffer(FramebufferState& fboState, const AppState& appState) {
	glGenFramebuffers(1, &fboState.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, fboState.framebuffer);

	glGenTextures(1, &fboState.textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, fboState.textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, appState.framebufferWidth, appState.framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboState.textureColorbuffer, 0);

	glGenRenderbuffers(1, &fboState.renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, fboState.renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appState.framebufferWidth, appState.framebufferHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fboState.renderbuffer);

	bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	if (!complete)
		DebugLog::log("[FRAMEBUFFER ERROR] Framebuffer is not complete! Kaput!!!!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return complete;
}

void resizeFramebufferAttachments(		
										FramebufferState& fboState, 
										AppState& appState
) {

	if (appState.framebufferWidth <= 0 || appState.framebufferHeight <= 0) {
		DebugLog::log("[FRAMEBUFFER WARNING] Ignored invalid resize dimensions");
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fboState.framebuffer);

	glBindTexture(GL_TEXTURE_2D, fboState.textureColorbuffer);

	glTexImage2D(
					GL_TEXTURE_2D, 
					0, 
					GL_RGB, 
					appState.framebufferWidth,
					appState.framebufferHeight, 
					0, 
					GL_RGB, 
					GL_UNSIGNED_BYTE, 
					nullptr
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, fboState.renderbuffer);

	glRenderbufferStorage(
							GL_RENDERBUFFER, 
							GL_DEPTH24_STENCIL8,
							appState.framebufferWidth, 
							appState.framebufferHeight
	);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		DebugLog::log("[FRAMEBUFFER ERROR] Framebuffer incomplete after resize");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	appState.framebufferResized = false;

	DebugLog::log("[FBO RESIZE] Framebuffer: " + std::to_string(appState.framebufferWidth) + "x" +
		std::to_string(appState.framebufferHeight)
	);

}

void destroyFramebuffer(FramebufferState& fboState) {
	if (fboState.renderbuffer != 0) {
		glDeleteRenderbuffers(1, &fboState.renderbuffer);
		fboState.renderbuffer = 0;
	}

	if (fboState.textureColorbuffer != 0) {
		glDeleteTextures(1, &fboState.textureColorbuffer);
		fboState.textureColorbuffer = 0;
	}

	if (fboState.framebuffer != 0) {
		glDeleteFramebuffers(1, &fboState.framebuffer);
		fboState.framebuffer = 0;
	}
}