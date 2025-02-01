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
#include <fstream>
#include <filesystem>

static constexpr char sText_JASC_PAL[] = "JASC-PAL";
static constexpr char sText_PAL_0100[] = "0100";

auto LoadPLTFromFile(const std::string &fname)
{
    std::ifstream stream(fname);
    std::vector<Color> colors;
    std::string line;

    uint16_t num_colors;
    int r, g, b;

    stream >> line;
    if (line != sText_JASC_PAL) 
        throw ("Invalid JASC-PAL signature.");

    stream >> line;
    if (line != sText_PAL_0100) 
        throw ("Unsupported JASC-PAL version.");

    if (!(stream >> num_colors))
        throw ("Could not parse number of colors.");

    if (num_colors < 1 || num_colors > 256)
        throw ("Unsupported number of colors. (Color count must be between 1 and 256)");

    for (int i = 0; i < num_colors; i++)
    {
        if (!(stream >> r >> g >> b))
            throw ("Error parsing color components.");

        if (r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
            throw ("Color component value must be between 0 and 255.");

        colors.push_back({ 
            static_cast<unsigned char>(r), 
            static_cast<unsigned char>(g), 
            static_cast<unsigned char>(b) 
        });
    }

    stream.close();

    return colors;
}

void load_tilemap(const std::string &fname)
{
    std::ifstream fs(fname, std::ios::binary);

    for (int i = 0; i < 0x400; ++i)
    {
        uint16_t entry;
        fs.read(reinterpret_cast<char *>(&entry), 2);
        global.tilemap[i] = entry;
    }

    fs.close();
}

void load_primary_tileset(const std::string &fname)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(fname.c_str(), &w, &h, &n, 4);

    if (!data || w != 128 || h != 256 || n != 1)
    {
        stbi_image_free(data);
        return;
    }

    renderer_load_primary(global.renderer, data);
    stbi_image_free(data);
}

void load_secondary_tileset(const std::string &fname)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(fname.c_str(), &w, &h, &n, 4);

    if (!data || w != 128 || h != 256 || n != 1)
    {
        stbi_image_free(data);
        return;
    }

    renderer_load_secondary(global.renderer, data);
    stbi_image_free(data);
}


static const char gPaletteFileNames[][7] =
{
    "00.pal",
    "01.pal",
    "02.pal",
    "03.pal",
    "04.pal",
    "05.pal",
    "06.pal",
    "07.pal",
    "08.pal",
    "09.pal",
    "10.pal",
    "11.pal",
    "12.pal",
    "13.pal",
    "14.pal",
    "15.pal"
};

void load_palettes(const std::string &s)
{
    std::filesystem::path basePalettePath = s;

    for (int i = 0; i < 16; ++i)
    {
        auto palPath = basePalettePath / gPaletteFileNames[i];

        if (std::filesystem::exists(palPath))
            renderer_load_palette(global.renderer, i, LoadPLTFromFile(palPath.string()).data());
    }

    renderer_change_palette(global.renderer, global.brush.palette);
}

static void mainmenubar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Tilemap", "Ctrl+O"))
            {
                std::string s;
                if (FileDialog::Open(FileDialog::Mode::Open, { {"Parallax", "bin"} }, s))
                    load_tilemap(s);
            }

            if (ImGui::MenuItem("Open Primary Tileset", nullptr))
            {
                std::string s;
                if (FileDialog::Open(FileDialog::Mode::Open, { {"Tileset", "png"} }, s))
                    load_primary_tileset(s);
            }

            if (ImGui::MenuItem("Open Secondary Tileset", nullptr))
            {
                std::string s;
                if (FileDialog::Open(FileDialog::Mode::Open, { {"Tileset", "png"} }, s))
                    load_secondary_tileset(s);
            }

            if (ImGui::MenuItem("Open Palettes", nullptr))
            {
                std::string s;
                if (FileDialog::Open(FileDialog::Mode::Folder, {}, s))
                    load_palettes(s);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

static void tileset_selector(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    ImTextureID tex_id = (ImTextureID)(uintptr_t)global.renderer.pickerFinalTex;
    ImVec2 scale = ImVec2(3, 3);
    ImVec2 size = ImVec2(128, 512);
    ImVec2 tilesize = ImVec2(8, 8);

    static int sTilesInRow = 16;

    for (int i = 0; i < 1024; ++i)
    {
        int x = i % sTilesInRow, y = i / sTilesInRow;
        ImVec2 uv0 = ImVec2(i % 16, i / 16) * tilesize / size;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddImage(tex_id, pos, pos + tilesize * scale, uv0, uv0 + tilesize / size);

        ImRect bb_ = ImRect(pos, pos + tilesize * scale);
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), &hovered, &held);

        if (held || pressed)
            global.brush.tile = i;
    }

    ImVec2 widgetsize = ImVec2(sTilesInRow, static_cast<int>(1024 / sTilesInRow)) * tilesize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(global.brush.tile % sTilesInRow, static_cast<int>(global.brush.tile / sTilesInRow)) * tilesize * scale;
    drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

static void tileset_pane(void)
{
    if (ImGui::BeginChild("Tileset", ImVec2(400.0f, 0.0f)))
    {
        int selected_tile = 16;
        bool xflip, yflip;

        ImGui::Text("ID: 0x%X", global.brush.tile);

        ImGui::Spacing();

        ImGui::Checkbox("X Flip", &global.brush.xflip); ImGui::SameLine();
        ImGui::Checkbox("Y Flip", &global.brush.yflip);

        ImGui::Spacing();

        int palettenum = global.brush.palette;
        if (ImGui::InputInt("Palette", &palettenum))
        {
            if (palettenum < 0) palettenum = 0;
            else if (palettenum > 15) palettenum = 15;
            global.brush.palette = palettenum;
            renderer_change_palette(global.renderer, global.brush.palette);
        }

        ImGui::Spacing();

        if (ImGui::BeginChild("###TilesetSelector", ImVec2(380.0f, 0.0f)))
        {
            tileset_selector();
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }
}

enum Mask : short
{
    Index = 0x3FF,
    FlipX = 0x400,
    FlipY = 0x800
};

static void right_click(unsigned short i)
{
    global.brush.xflip = (i & Mask::FlipX) == Mask::FlipX;
    global.brush.yflip = (i & Mask::FlipY) == Mask::FlipY;
    global.brush.tile = i & Mask::Index;
    
    global.brush.palette = (i >> 12) & 0xF;
    renderer_change_palette(global.renderer, global.brush.palette);
}

static void left_click(unsigned int i)
{
    uint16_t newEntry = (
        global.brush.tile & Mask::Index |
        ((global.brush.palette & 0xF) << 12) |
        (global.brush.xflip ? Mask::FlipX : 0) |
        (global.brush.yflip ? Mask::FlipY : 0)
    );
    global.tilemap[i] = newEntry;
}

static void tilemap_panel(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    ImTextureID tex_id = (ImTextureID)(uintptr_t)global.renderer.mapFinalTex;
    ImVec2 scale = ImVec2(4, 4);
    ImVec2 size = ImVec2(256, 256);
    ImVec2 tilesize = ImVec2(8, 8);

    static constexpr int sTilesInRow = 32;
    bool has_hovered = false;
    int hovered_item;
    static bool changed_tile = true;
    static int last_hovered_item;

    for (int i = 0; i < 1024; ++i)
    {
        int x = i % sTilesInRow, y = i / sTilesInRow;
        ImVec2 uv0 = ImVec2(i % 32, i / 32) * tilesize / size;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddImage(tex_id, pos, pos + tilesize * scale, uv0, uv0 + tilesize / size);

        ImRect bb_ = ImRect(pos, pos + tilesize * scale);
        bool hovered = ImGui::ItemHoverable(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), ImGuiItemFlags_AllowOverlap);

        if (hovered)
        {
            has_hovered = true;
            hovered_item = i;

            if (last_hovered_item != hovered_item)
                changed_tile = true;
            
            last_hovered_item = i;

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) 
                right_click(global.tilemap[i]);
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) 
            {
                if (changed_tile) left_click(i);
                changed_tile = false;
            }
        }
    }

    if (has_hovered)
    {
        int x = hovered_item % 32, y = hovered_item / 32;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
    }

    ImVec2 widgetsize = ImVec2(sTilesInRow, static_cast<int>(1024 / sTilesInRow)) * tilesize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

static void tilemap_pane(void)
{
    if (ImGui::BeginChild("Tilemap"))
    {
        tilemap_panel();
        // ImGui::Image(global.renderer.mapFinalTex, ImVec2(1024.0, 1024.0));
        ImGui::EndChild();
    }
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

        renderer_call(global.renderer);

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

                mainmenubar();
                tileset_pane();

                ImGui::SameLine();

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