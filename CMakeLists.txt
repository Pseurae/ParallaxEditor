cmake_minimum_required(VERSION 3.29)
project(ParallaxEditor VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external)

add_executable(${PROJECT_NAME}
    source/Global.cpp
    source/FileDialog.cpp
    source/ParallaxEditor.cpp
    source/Renderer.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui glfw gl3w stb_image nfd)