#include "include/ShaderSystem.h"

#include "include/FileSystem.h"

ShaderState::ShaderState() :
	sceneShader(
		FileSystem::pathFromRoot("src/shaders/vertex_shader.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/fragment_shader.frag").c_str()
	),
	screenShader(
		FileSystem::pathFromRoot("src/shaders/fbuffer_vertex_shader.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/fbuffer_fragment_shader.frag").c_str()
	),
	uiShader(
		FileSystem::pathFromRoot("src/shaders/note_quad.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/note_quad.frag").c_str()
	),
	textShader(
		FileSystem::pathFromRoot("src/shaders/text.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/text.frag").c_str()
	),
	uiRectangleShader(
		FileSystem::pathFromRoot("src/shaders/ui_rectangle.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/ui_rectangle.frag").c_str()
	)
{}

void initializeShaders(ShaderState& shaders) {
	shaders.screenShader.use();
	shaders.screenShader.setFloat("vignetteStrength", 0.5f);
	shaders.screenShader.setFloat("grainStrength", 0.02f);
	shaders.screenShader.setFloat("saturation", 0.7f);
	shaders.screenShader.setFloat("contrast", 1.2f);

	shaders.uiShader.use();
	shaders.uiShader.setInt("uiTexture", 0);
}
