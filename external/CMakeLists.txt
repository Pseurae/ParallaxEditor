add_subdirectory(imgui)
add_subdirectory(stb_image)

include(FetchContent)

FetchContent_Declare(glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw"
    GIT_TAG 3.4
)

set(GLFW_BUILD_DOCS FALSE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(gl3w 
    GIT_REPOSITORY "https://github.com/adobe/lagrange-gl3w" 
    GIT_TAG "main"
)

cmake_policy(SET CMP0148 OLD)
FetchContent_MakeAvailable(gl3w)

FetchContent_Declare(nfd
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended"
    GIT_TAG v1.2.1
)

FetchContent_MakeAvailable(nfd)
