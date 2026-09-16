#include "include/TextureUtilities.h"

#include <glad/glad.h>
#include "stb_image.h"

#include "include/DebugLog.h"

namespace {

GLenum formatFromChannels(int channels){
	switch(channels) {

	case 1:
		return GL_RED;

	case 2:
		return GL_RG;

	case 3:
		return GL_RGB;

	case 4:
		return GL_RGBA;

	default:
		return 0;

	}
}

unsigned int createTexture2D(const char* path, bool flipVertically, GLint wrapMode){
	stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned char* pixels = stbi_load(path, &width, &height, &channels, 0);

	if (!pixels) {
		DebugLog::log(std::string("[TEXTURE ERROR] Failed to load: ") + path);
		return 0;
	}
	
	GLenum format = formatFromChannels(channels);

	if (format == 0) {
		DebugLog::log(std::string("[TEXTURE ERROR] Unsupported format: ") + path);
		stbi_image_free(pixels);
		return 0;
	}

	unsigned int texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width,
		height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		pixels
	);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return texture;
}

} //end of namespace

unsigned int loadTexture(const char *path) { 	
	return createTexture2D(path, false, GL_REPEAT);
}

unsigned int loadTexture2D(const char* path){
	return createTexture2D(path, true, GL_CLAMP_TO_EDGE);
}

// loads a cubemap texture from 6 individual texture faces in order:
//+X (right) -X (left)
//+Y (top) -Y (bottom)
//+Z (front) -Z (back)
unsigned int loadCubemap(std::vector<std::string> faces) {
	
	if (faces.size() != 6) {
		DebugLog::log("[CUBEMAP ERROR] Expected 6 faces.");
		return 0;
	}

	stbi_set_flip_vertically_on_load(false);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	for (std::size_t i = 0; i < faces.size(); i++) {
		int width = 0;
		int height = 0;
		int channels = 0;
		
		unsigned char* pixels = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

		if (!pixels) {
			DebugLog::log("[CUBEMAP ERROR] Failed to load texture: " + faces[i]);
			glDeleteTextures(1, &texture);
			return 0;
		}

		GLenum format = formatFromChannels(channels);

		if (format == 0) {
			DebugLog::log("[CUBEMAP ERROR] Unsupported format: " + faces[i]);
			
			stbi_image_free(pixels);
			glDeleteTextures(1, &texture);
			return 0;
		}

		glTexImage2D(	GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				0, 
				GL_RGB, 
				width,
				height, 
				0, 
				GL_RGB, 
				GL_UNSIGNED_BYTE, 
				pixels
		);

		stbi_image_free(pixels);
	}	

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	return texture;
}

