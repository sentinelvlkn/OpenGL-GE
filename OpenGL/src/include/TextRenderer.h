#pragma once

#include <map>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.h"

struct Character {
	unsigned int textureID;
	glm::vec2 size;
	glm::vec2 bearing;
	unsigned int advance;
};

class TextRenderer {
public:
	TextRenderer();
	~TextRenderer();

	bool loadFont(const std::string &fontPath, unsigned int fontSize);

	void renderText(Shader &shader, const std::string &text, float x, float y,
					float scale, const glm::vec3 &color, int screenWidth,
					int screenHeight);
	float measureTextWidth(const std::string &text, float scale) const;

private:
	std::map<char, Character> characters;
	unsigned int VAO = 0;
	unsigned int VBO = 0;

};
