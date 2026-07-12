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

	if (length(texColor) < 0.01)
		texColor = vec3(1.0);


	vec3 normal = Normal;

	if (length(normal) < 0.001)
		normal = vec3(0.0, 1.0, 0.0);
	else
		normal = normalize(normal);

	vec3 ambient = ambientColor * texColor;

	vec3 toLight = lightPos - FragPos;
	float distanceToLight = length(toLight);
	
	vec3 lightDir = vec3(0.0, 1.0, 0.0);
	if (distanceToLight > 0.0001)
		lightDir = toLight / distanceToLight;

	//float diff = max(dot(normal, lightDir), 0.0);
	//float diff = abs(dot(normal, lightDir));
	float diff = 1.0;
	
	float radius = max(lightRadius, 0.001);
	float attenuation = 1.0 - smoothstep(0.0, radius, distanceToLight);
	attenuation = clamp(attenuation, 0.0, 1.0);

	vec3 diffuse = diff * lightColor * lightIntensity * attenuation * texColor;

	vec3 result = ambient + diffuse;
	result = clamp(result, 0.0, 1.0);

	FragColor = vec4(result, texSample.a);
}
