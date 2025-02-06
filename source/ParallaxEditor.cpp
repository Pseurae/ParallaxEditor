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
#include "Shortcut.h"

void load_tilemap_from_file(const std::string &fname);
void load_secondary_tileset(const std::string &fname);
void load_palettes(const std::string &s);

void status_bar(void)
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    auto barPos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - ImGui::GetFrameHeight());
    auto barSize = ImVec2(viewport->Size.x, ImGui::GetFrameHeight());

    ImGui::SetNextWindowPos(barPos);
    ImGui::SetNextWindowSize(barSize);

    static constexpr ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("###StatusBar", nullptr, flags);

    ImGui::SetCursorPosX(16.0f);
    if (ImGui::BeginMenuBar())
    {
        static char zoomLabel[15];
        snprintf(zoomLabel, 15, "Zoom: %i%%", (int)(global.zoomScale * 100.0f));

        if (ImGui::BeginMenu(zoomLabel))
        {
            if (ImGui::MenuItem("100%")) global.zoomScale = 1.0f;
            if (ImGui::MenuItem("150%")) global.zoomScale = 1.5f;
            if (ImGui::MenuItem("200%")) global.zoomScale = 2.0f;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();

    ImGui::PopStyleVar();
}

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

    glfwSetKeyCallback(window, [](GLFWwindow* window, int k, int, int a, int m) {
        shortcut_callback(k, m, a);
    });

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
    load_tilemap_from_file("tamarok.bin");

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
                ImGui::SetNextWindowSize(viewport->Size - ImVec2(0.0f, ImGui::GetFrameHeight()));

                static constexpr ImGuiWindowFlags sWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse;
                ImGui::Begin("###ParallaxEditor", NULL, sWindowFlags | ImGuiWindowFlags_MenuBar);

                main_menu_bar();
                tileset_pane(); ImGui::SameLine();
                tilemap_pane();

                ImGui::End();

                status_bar();
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