/*
 * FileSystem.h
 *
 *  Created on: Jan 10, 2026
 *      Author: volokin
 */

#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_

#include <string>

class FileSystem
{
public:
    // Returns a usable path relative to the project root
    // Example: FileSystem::getPath("src/shaders/vertex_shader.vert")
    static std::string getPath(const std::string& path)
    {
        return path;
    }
};

#endif /* SRC_FILESYSTEM_H_ */
