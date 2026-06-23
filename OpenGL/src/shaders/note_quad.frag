#version 420 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D uiTexture;

void main() {
	vec4 texColor = texture(uiTexture, TexCoords);
	
	if (texColor.a < 0.01)
		discard;

	FragColor = texColor;
}
