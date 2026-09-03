#pragma once

struct AppState;

struct FramebufferState {
	unsigned int framebuffer = 0;
	unsigned int textureColorbuffer = 0;
	unsigned int renderbuffer = 0;
};

bool initializeFramebuffer(FramebufferState& fboState, const AppState& appState);

void resizeFramebufferAttachments(FramebufferState& fboState, AppState& appState);

void destroyFramebuffer(FramebufferState& fboState);

