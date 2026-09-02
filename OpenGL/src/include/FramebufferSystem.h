#pragma once

struct AppState;

void resizeFramebufferAttachments(	
									unsigned int fbo,
									unsigned int textureColorBuffer,
									unsigned int rbo, 
									AppState& appState
);

