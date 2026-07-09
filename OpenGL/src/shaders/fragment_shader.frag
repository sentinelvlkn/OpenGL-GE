#version 420 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

uniform vec3 ambientColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float lightRadius;

void main() {    
	
	vec4 texSample = texture(texture_diffuse1, TexCoords);
	
	if (texSample.a < 0.05)
		discard;
	
	vec3 texColor = texSample.rgb;

	vec3 normal = normalize(Normal);

	vec3 ambient = ambientColor * texColor;

	vec3 toLight = lightPos - FragPos;
	float distanceToLight = length(toLight);
	vec3 lightDir = normalize(toLight);

	float diff = max(dot(normal, lightDir), 0.0);

	float attenuation = 1.0 - smoothstep(0.0, lightRadius, distanceToLight);
	attenuation = clamp(attenuation, 0.0, 1.0);

	vec3 diffuse = diff * lightColor * lightIntensity * attenuation * texColor;

	vec3 result = ambient + diffuse;

	FragColor = vec4(result, texSample.a);
}
