#version 420 core

out vec4 FragColor;

uniform vec3 rectangleColor;
uniform float rectangleAlpha;

void main() {

	FragColor = vec4(rectangleColor, rectangleAlpha);

}