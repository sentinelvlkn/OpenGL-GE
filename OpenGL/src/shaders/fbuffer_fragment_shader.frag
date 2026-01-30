#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float fade;

void main(){
	vec3 col =  texture(screenTexture, TexCoords).rgb;
	col *= fade;
	FragColor = vec4(col, 1.0);
	//FragColor = texture(screenTexture, TexCoords);

}