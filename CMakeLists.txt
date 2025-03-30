cmake_minimum_required(VERSION 3.29)
project(ParallaxEditor VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external)

add_executable(${PROJECT_NAME}
    source/Core/Context.cpp
    source/Core/Popup.cpp
    source/Core/Renderer.Constants.cpp
    source/Core/Renderer.cpp

    source/UI/Popups/Error.cpp
    source/UI/Popups/NewTilemap.cpp
    source/UI/Popups/Palettes.cpp
    source/UI/Popups/Prompt.cpp

    source/UI/Helpers.cpp
    source/UI/MainWindow.cpp
    source/UI/MenuBar.cpp
    source/UI/TilemapPane.cpp
    source/UI/TilesetPane.cpp

    source/Utils/FileDialog.cpp
    source/Utils/Palette.cpp

    source/Global.cpp
    source/ParallaxEditor.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC imgui glfw gl3w stb_image nfd tomlplusplus::tomlplusplus)
target_include_directories(${PROJECT_NAME} PUBLIC source)