#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main(){    
    vec4 texColor = texture(texture_diffuse1, TexCoords);
	if(texColor.a < 0.05) discard;
	FragColor = texColor;
	
	//FragColor = texture(texture_diffuse1, TexCoords);
}