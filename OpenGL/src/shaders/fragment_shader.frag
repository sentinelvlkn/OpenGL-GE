#version 420 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 ambientColor;

void main(){    
	vec4 texColor = texture(texture_diffuse1, TexCoords);

	if(texColor.a < 0.05) 
		discard;

	vec3 litColor = texColor.rgb * ambientColor;

	FragColor = vec4(litColor, texColor);


	//FragColor = texture(texture_diffuse1, TexCoords);
}
