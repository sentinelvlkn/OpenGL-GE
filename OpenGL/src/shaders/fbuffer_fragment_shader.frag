#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float fade;
uniform float time;

//controls for effects
uniform float vignetteStrength;
uniform float grainStrength;
uniform float saturation;
uniform float contrast;
uniform float brightness;

float random(vec2 p) {
	return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 applySaturation(vec3 color, float sat) {
	float gray = dot(color, vec3(0.299, 0.587, 0.114));
	return mix(vec3(gray), color, sat);
}

vec3 applyContrast(vec3 color, float c) {
	return (color - 0.5) * c + 0.5;
}

void main() {
	vec3 color = texture(screenTexture, TexCoords).rgb;

	//light grain
	float grain = random(TexCoords * time * 1200.0);
	color += (grain - 0.5) * grainStrength;

	//desaturation
	color = applySaturation(color, saturation);

	//contrast & brightness
	color = applyContrast(color, contrast);
	color += brightness;

	//vignette
	vec2 centered = TexCoords - vec2(0.5);
	float dist = length(centered);

	float vignette = smoothstep(0.8, 0.25, dist);
	color *= mix(1.0, vignette, vignetteStrength);

	//fade to black
	color = mix(color, vec3(0.0), fade);

	FragColor = vec4(color, 1.0);
}
