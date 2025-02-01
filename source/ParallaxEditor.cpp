#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "ParallaxEditor.h"
#include "Global.h"
#include "FileDialog.h"
#include "Utils.h"
#include "Pane.Map.h"
#include "Pane.Picker.h"
#include "MenuBar.h"

int main(int argc, char *argv[])
{
    ASSERT(glfwInit(), "GLFW could not be initialized.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    ASSERT(window, "GLFWwindow could not be created.");

    {
        float dpiScaleX, dpiScaleY;
        glfwGetWindowContentScale(window, &dpiScaleX, &dpiScaleY);
        global.dpiScale = dpiScaleX > dpiScaleY ? dpiScaleX : dpiScaleY;
    }

    glfwSetWindowSize(window, 800 * global.dpiScale, 600 * global.dpiScale);
    glfwMakeContextCurrent(window);

    glfwShowWindow(window);

    ASSERT(gl3wInit() != -1, "GL3W could not be initialized.");

    FileDialog::Init(window);

    renderer_init(global.renderer);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.FontGlobalScale *= global.dpiScale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    load_secondary_tileset("tiles.png");
    load_palettes("palettes");
    load_tilemap("tamarok.bin");

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer_call(global.renderer, global.tilemap);

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                ImGuiViewport *viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);

                static constexpr ImGuiWindowFlags sWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse;
                ImGui::Begin("Window", NULL, sWindowFlags | ImGuiWindowFlags_MenuBar);

                main_menu_bar();
                tileset_pane(); ImGui::SameLine();
                tilemap_pane();

                ImGui::End();
            }

            int displayW, displayH;
            ImGui::Render();
            glfwGetFramebufferSize(window, &displayW, &displayH);
            glViewport(0, 0, displayW, displayH);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}