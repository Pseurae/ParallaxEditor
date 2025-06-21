#include "UI/OptionsPane.h"
#include <filesystem>
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"

void OptionsPane(void)
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0, 0.0), ImVec2(FLT_MAX, 600.0f));
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SliderFloat("Opacity", &global.transparency, 0.0, 1.0);
    ImGui::Spacing();

    {
        for (const auto &groupName : global.context.GetMapOrderLabels())
        {
            if (ImGui::TreeNode(groupName.c_str()))
            {
                for (const auto &mapName : global.context.GetGroupedMapLabels().at(groupName))
                {
                    if (ImGui::Selectable(mapName.c_str()))
                    {
                        TryOpenMap(mapName);
                    }
                }
                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}