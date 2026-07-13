#version 420 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

uniform vec3 ambientColor;

#define MAX_POINT_LIGHTS 8

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float radius;
};

uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

void main() {    
	
	vec4 texSample = texture(texture_diffuse1, TexCoords);
	
	if (texSample.a < 0.05)
		discard;
	
	vec3 texColor = texSample.rgb;

	if (length(texColor) < 0.01)
		texColor = vec3(1.0);

	vec3 ambient = ambientColor * texColor;

	vec3 totalDiffuse = vec3(0.0);

	for (int i = 0; i < numPointLights; ++i) {
		vec3 toLight = pointLights[i].position - FragPos;
		float distanceToLight = length(toLight);

		float radius = max(pointLights[i].radius, 0.001);
		float attenuation = 1.0 - smoothstep(0.0, radius, distanceToLight);
		attenuation = clamp(attenuation, 0.0, 1.0);
		
		//temporary diffuse value
		float diff = 1.0;

		vec3 diffuse = diff * pointLights[i].color * pointLights[i].intensity * attenuation * texColor;

		totalDiffuse += diffuse;
	}

	vec3 result = ambient + totalDiffuse;
	result = clamp(result, 0.0, 1.0);
	
	FragColor = vec4(result, texSample.a);
}
