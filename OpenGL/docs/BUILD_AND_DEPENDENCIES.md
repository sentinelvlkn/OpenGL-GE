# Build and dependencies

What this project depends on and how to build it

For engine structure, see:
```text
docs/CODE_STRUCTURE.md
```

---

## Main dependencies

The engine uses:
```text
C++17
CMake
OpenGL 4.2 core
GLFW, GLAD and GLM
Assimp
stb_image
nlohmann/json
FreeType
```

---

## Project root

The engine uses `PROJECT_ROOT_DDIR` so that code can build file path relative to project root.

---

## Windows build

Recommended setup:
```text
Visual studio
CMake
vcpkg
```

Example configure command:
```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Example build comman:
```powershell
cmake --build build --config Debug
```

Run from VS or from the generated build output.

---

## Linux build

Example config command:
```bash
cmake -S . -B build
```

Example build command:
```bash
cmake --build build
```

Run command:
```bash
.build/OpenGLGame
```

Depending on your Linux setup, you may need development packages for:
```text
GLFW
OpenGL
Assimp
FreeType
CMake
compiler toolchain
```

---

## How header includes should be ordered

- glad should be always included before glfw
```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
```

- keep this cmake definition
```cmake
GLFW_INCLUDE_NONE
```
this prevents glfw from including opengl headers before glad

---

## Source files in CMake

When adding a new `.cpp` file, include it in the build to be compiled.
If a function exists inside a header but the linker cannot find it, check that the matching `.cpp` file is part of the cmake target.

---

## Asset path

Asset paths should be written relative to the project root.
```cpp
FileSystem::pathFromRoot("src/models/test_room/test_room.json");
```

Existing asset folders:
```text
src/models/
src/shaders/
src/ui/
src/ui/fonts/
src/ui/notes/
```

---

## Common build errors

### GLAD / GLFW include conflict

Problem:
```text
OpenGL headers included before GLAD
```

Check that glad is included before glfw:
```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
```
and:
```cmake
GLFW_INCLUDE_NONE
```

---

## Unresolved external symbol

Problem:
```text
unresolved external symbol someFunctionName
```

Usually means one of these is wrong:
```text
1. The .cpp file is not included in CMake.
2. The function signature in the header and cpp do not match.
3. The function name is misspelled.
```

---

## Redefinition of default argument

Problem:
```text
C2572: redefinition of default argument
```

Default arguments should be written in the header only.

Function declaration inside a header with default arguments:
```cpp
bool initializeWindow(
    WindowState& windowState,
    AppState& appState,
    GameState& gameState,
    AppContext& appContext,
    int width,
    int height,
    const char* title,
    bool enableMouseLook = false
);
```

How the same function should be defined in the .cpp:
```cpp
bool initializeWindow(
    WindowState& windowState,
    AppState& appState,
    GameState& gameState,
    AppContext& appContext,
    int width,
    int height,
    const char* title,
    bool enableMouseLook
)
```

---

## File not found at runtime

Problem:
```text
failed to load [some path like "shader/model/font/texture"]
```

Check:
```text
1. The path is relative to the project root.
2. PROJECT_ROOT_DIR is defined in CMake.
3. The file exists at the expected location.
4. The working directory is not being assumed incorrectly.
```

---

## Clean rebuild

Windows PowerShell:
```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
```

Linux/macOS shell:
```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

