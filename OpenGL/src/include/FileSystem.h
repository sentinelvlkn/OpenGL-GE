#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_

#include <string>
#include <filesystem>

class FileSystem
{
public:
    // Returns a usable path relative to the project root
    // Example: FileSystem::getPath("src/shaders/vertex_shader.vert")
    static std::string getPath(const std::string& path)
    {
        return path;
    }
    
    static std::string pathFromRoot(const std::string &loc) {
		return (std::filesystem::path(PROJECT_ROOT_DIR) / loc).string();
    }
};


#endif /* SRC_FILESYSTEM_H_ */
