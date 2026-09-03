cmake_minimum_required(VERSION 3.16)
project(OpenGLGame LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sources
add_executable(${PROJECT_NAME}
    src/source.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Shader.cpp
    src/TextRenderer.cpp
    src/UiRenderer.cpp
    src/DebugLog.cpp
    src/Input.cpp
    src/PlayerMovement.cpp
    src/InteractionSystem.cpp
    src/DebugTerminal.cpp
    src/AssetCache.cpp
    src/LightingSystem.cpp
    src/TextureUtilities.cpp
    src/FadeSystem.cpp
    src/FramebufferSystem.cpp
    src/ScreenRenderer.cpp
    src/SceneRenderer.cpp
    src/RoomTransitionSystem.cpp
    src/CameraSystem.cpp
    src/GameplaySystem.cpp
    src/GameUIRenderer.cpp
    src/DebugInputSystem.cpp
    src/FrameTimingSystem.cpp
    src/GameBootstrap.cpp
    src/ShaderSystem.cpp

    src/stb_image_impl.cpp
  "src/glad.c" "src/include/Room.h" "src/Room.cpp" "src/include/Prop.h")

#includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
    GLFW_INCLUDE_NONE
)

set_property(TARGET ${PROJECT_NAME}
  PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

#OpenGL
find_package(OpenGL REQUIRED)

#External libs (install via vcpkg on Windows, apt on Linux)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    assimp::assimp
    Freetype::Freetype
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
