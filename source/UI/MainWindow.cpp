#include "UI/MainWindow.h"
#include "UI/TilesetPane.h"
#include "UI/TilemapPane.h"
#include "Global.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "UI/Popups/Prompt.h"

void MainWindow(void)
{
    if (!global.contexts.empty())
    {
        if (ImGui::BeginTabBar("##OpenedFiles", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable))
        {
            global.contexts.remove_if([](auto &ctx) {
                bool isOpen = true;
                int flags = (ctx->IsDirty() ? ImGuiTabItemFlags_UnsavedDocument : 0) | ImGuiTabItemFlags_NoTooltip;

                ImGui::PushID(ctx.get());
                if (ImGui::BeginTabItem(ctx->GetName().c_str(), &isOpen, flags))
                {
                    global.currentContext = ctx.get();
                    ImGui::EndTabItem();
                }

                ImGui::PopID();

                if (!isOpen)
                {
                    if (ctx->IsDirty())
                        global.popupManager.Open<Popups::Prompt>("This file contains unsaved changes.\nClose anyways?", 
                            [&ctx](){ global.contexts.remove(ctx); }, nullptr);
                    else 
                        return true;
                }

                return false;
            });

            ImGui::EndTabBar();
        }

        if (global.currentContext)
        {
            TilesetPane();
            ImGui::SameLine();
            TilemapPane();
        }
    }
}
