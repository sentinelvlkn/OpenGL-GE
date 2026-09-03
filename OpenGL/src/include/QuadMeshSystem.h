#pragma once

struct QuadMeshState {
	unsigned int vao = 0;
	unsigned int vbo = 0;
};

void initializeScreenQuad(QuadMeshState& quadMeshState);

void destroyScreenQuad(QuadMeshState& quadMeshState);