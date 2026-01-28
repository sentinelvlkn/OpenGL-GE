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
    src/glad.c
    src/stb_image_impl.cpp
)

#includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src
)

#OpenGL
find_package(OpenGL REQUIRED)

#External libs (install via vcpkg on Windows, apt on Linux)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    assimp::assimp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
