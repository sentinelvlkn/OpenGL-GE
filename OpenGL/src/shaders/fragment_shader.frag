#version 420 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

uniform vec3 ambientColor;

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 4

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float radius;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 color;
	float radius;
	float intensity;
	float innerCutoff;
	float outerCutoff;
};

uniform int numPointLights;
uniform int numSpotLights;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

float distanceAttenuation(float distanceToLight, float radius) {
	float r = max(radius, 0.001);
	float attenuation = 1.0 - smoothstep(0.0, r, distanceToLight);
	return clamp(attenuation, 0.0, 1.0);
}

void main() {    
	
	vec4 texSample = texture(texture_diffuse1, TexCoords);
	
	if (texSample.a < 0.05)
		discard;
	
	vec3 texColor = texSample.rgb;

	if (length(texColor) < 0.01)
		texColor = vec3(1.0);

	vec3 ambient = ambientColor * texColor;

	vec3 totalDiffuse = vec3(0.0);

	// point lights
	for (int i = 0; i < numPointLights; ++i) {
		vec3 toLight = pointLights[i].position - FragPos;
		float distanceToLight = length(toLight);

		float radius = max(pointLights[i].radius, 0.001);
		float attenuation = 1.0 - smoothstep(0.0, radius, distanceToLight);
		attenuation = clamp(attenuation, 0.0, 1.0);
		
		//temporary diffuse value
		float diff = 1.0;

		vec3 diffuse = diff * 
						pointLights[i].color * 
						pointLights[i].intensity * 
						attenuation * 
						texColor;

		totalDiffuse += diffuse;
	}

	// spot lights
	for (int i = 0; i < numSpotLights; ++i) {
		vec3 toLight = spotLights[i].position - FragPos;
		float distanceToLight = length(toLight);

		float attenuation = distanceAttenuation(distanceToLight, spotLights[i].radius);

		//direction from light to fragment
		vec3 lightToFragDir = normalize(FragPos - spotLights[i].position);

		if (length(lightToFragDir) < 0.0001)
			lightToFragDir = spotLights[i].direction; 
		else
			lightToFragDir = normalize(lightToFragDir);

		vec3 spotDir = normalize(spotLights[i].direction);

		//cone test
		float theta = dot(lightToFragDir, spotDir);

		float inner = cos(radians(spotLights[i].innerCutoff));
		float outer = cos(radians(spotLights[i].outerCutoff));

		float epsilon = max(inner - outer, 0.001);
		float cone = clamp((theta - outer) / epsilon, 0.0, 1.0);

		float diff = 1.0;

		vec3 diffuse = diff *
				spotLights[i].color *
				spotLights[i].intensity *
				attenuation *
				cone *
				texColor;

		totalDiffuse += diffuse;
	}

	vec3 result = ambient + totalDiffuse;
	result = clamp(result, 0.0, 1.0);
	
	FragColor = vec4(result, texSample.a);
}
