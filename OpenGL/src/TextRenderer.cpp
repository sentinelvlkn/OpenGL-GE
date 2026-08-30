#include "include/TextRenderer.h"

#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/gtc/matrix_transform.hpp>
#include "include/DebugLog.h"

TextRenderer::TextRenderer() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(	GL_ARRAY_BUFFER, 
					sizeof(float) * 6 * 4, 
					nullptr,
					GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(	0, 
							4, 
							GL_FLOAT, 
							GL_FALSE, 
							4 * sizeof(float),
							(void *)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
	for (auto &pair : characters)
		glDeleteTextures(1, &pair.second.textureID);
	

	if (VBO != 0)
		glDeleteBuffers(1, &VBO);

	if (VAO != 0)
		glDeleteVertexArrays(1, &VAO);
}

bool TextRenderer::loadFont(const std::string &fontPath,
							unsigned int fontSize) {
	characters.clear();

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		DebugLog::log("[FREETYPE ERROR] Could not initialize FreeType");
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		DebugLog::log(std::string("[FREETYPE ERROR]: Failed to load font: ") + fontPath);
		FT_Done_FreeType(ft);
		return false;
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			DebugLog::log("[FREETYPE ERROR] Failed to load glyph: " +std::to_string(static_cast<int>(c)));
			continue;
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(	GL_TEXTURE_2D, 
						0, 
						GL_RED, 
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows, 
						0, 
						GL_RED, 
						GL_UNSIGNED_BYTE,
						face->glyph->bitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character;
		character.textureID = texture;
		character.size =
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.bearing =
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.advance = static_cast<unsigned int>(face->glyph->advance.x);

		characters.insert(std::pair<char, Character>(c, character));
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	DebugLog::log(std::string("[TEXT] Loaded font: ") + fontPath);
	return true;
}

void TextRenderer::renderText(Shader &shader, const std::string &text, float x,
							  float y, float scale, const glm::vec3 &color,
							  int screenWidth, int screenHeight) {
	if (text.empty())
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader.use();

	glm::mat4 projection = glm::ortho(	0.0f, 
										static_cast<float>(screenWidth),
										0.0f, 
										static_cast<float>(screenHeight));

	shader.setMat4("projection", projection);
	shader.setVec3("textColor", color);
	shader.setInt("text", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	for (char c : text) {
		auto it = characters.find(c);

		if (it == characters.end())
			continue;

		const Character &ch = it->second;

		float xpos = x + ch.bearing.x * scale;
		float ypos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;

		float vertices[6][4] = {
			{xpos, ypos + h, 0.0f, 0.0f},	 
			{xpos, ypos, 0.0f, 1.0f},
			{xpos + w, ypos, 1.0f, 1.0f},

			{xpos, ypos + h, 0.0f, 0.0f},	 
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos + w, ypos + h, 1.0f, 0.0f}};

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

float TextRenderer::measureTextWidth(const std::string& text, float scale) const {
	
	float width = 0.0f;

	for (char c : text) {
		auto it = characters.find(c);

		if (it == characters.end())
			continue;

		const Character &ch = it->second;
		width += (ch.advance >> 6) * scale;
	}

	return width;
}

