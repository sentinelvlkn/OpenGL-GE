#include "include/QuadMeshSystem.h"

#include <glad/glad.h>

#include "include/test_vertices.h"

void initializeScreenQuad(QuadMeshState& quadMeshState) {
	
	glGenVertexArrays(1, &quadMeshState.vao);
	glGenBuffers(1, &quadMeshState.vbo);

	glBindVertexArray(quadMeshState.vao);

	glBindBuffer(GL_ARRAY_BUFFER, quadMeshState.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void destroyScreenQuad(QuadMeshState& quadMeshState) {

	if (quadMeshState.vbo != 0) {
		glDeleteBuffers(1, &quadMeshState.vbo);
		quadMeshState.vbo = 0;
	}

	if (quadMeshState.vao != 0) {
		glDeleteVertexArrays(1, &quadMeshState.vao);
		quadMeshState.vao = 0;
	}

}