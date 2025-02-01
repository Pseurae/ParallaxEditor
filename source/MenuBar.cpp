#include "MenuBar.h"
#include "FileDialog.h"
#include <imgui.h>
#include "Utils.h"

void main_menu_bar(void)
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