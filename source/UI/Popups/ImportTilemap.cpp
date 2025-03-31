#include "UI/Popups/ImportTilemap.h"
#include <imgui.h>
#include "Utils/FileDialog.h"
#include "UI/Helpers.h"
#include <filesystem>

namespace Popups
{
void ImportTilemap::DrawContent(void)
{
    if (ImGui::Button("Open Tilemap"))
    {
        if (FileDialog::Open(FileDialog::Mode::Open, {{ "Tilemap", "bin" }}, mPath))
            mTiles = LoadBinaryTilemap(mPath);
    }

    ImGui::SameLine();
    ImGui::Text("%s", mPath.empty() ? "No file selected." : std::filesystem::path(mPath).filename().string().c_str());

    if (ImGui::InputInt("Width", &mWidth))
        mWidth = std::max(1, mWidth);

    ImGui::Text("The import tileset is gonna be %dx%d tiles. (%d tiles pruned)", 
            mWidth, (int)(mTiles.size() / mWidth), (int)(mTiles.size() % mWidth));

    if (ImGui::Button("Open"))
    {
        TryImportTilemap(mTiles, mWidth);
        Close();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        Close();
    }
}
}