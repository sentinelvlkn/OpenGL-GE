#include "include/FramebufferSystem.h"

#include <glad/glad.h>

#include <string>

#include "include/DebugLog.h"

void resizeFramebufferAttachments(		
										unsigned int fbo,
										unsigned int textureColorBuffer,
										unsigned int rbo, 
										int width,
										int height
) {

	if (width <= 0 || height <= 0) {
		DebugLog::log("[FRAMEBUFFER WARNING] Ignored invalid resize dimensions");
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	glTexImage2D(
					GL_TEXTURE_2D, 
					0, 
					GL_RGB, 
					width,
					height, 
					0, 
					GL_RGB, 
					GL_UNSIGNED_BYTE, 
					nullptr
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(
							GL_RENDERBUFFER, 
							GL_DEPTH24_STENCIL8,
							width, 
							height
	);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		DebugLog::log("[FRAMEBUFFER ERROR] Framebuffer incomplete after resize");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	DebugLog::log("[FBO RESIZE] Framebuffer: " + std::to_string(width) + "x" +
		std::to_string(height)
	);

}