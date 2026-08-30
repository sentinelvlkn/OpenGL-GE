#pragma once

#include <string>
#include <vector>

unsigned int loadTexture(const char *path);

unsigned int loadCubemap(std::vector<std::string> faces);

unsigned int loadTexture2D(const char *path);