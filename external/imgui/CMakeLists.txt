add_library(imgui 
    imgui_draw.cpp 
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
)

target_include_directories(imgui PUBLIC .)
target_link_libraries(imgui PUBLIC glfw)