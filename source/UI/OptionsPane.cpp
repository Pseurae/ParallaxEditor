#include "UI/OptionsPane.h"
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"

void OptionsPane(void)
{
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("Load Underlay"))
        TryLoadUnderlay();
    if (ImGui::Button("Remove Underlay"))
        global.renderer.LoadEmptyUnderlay();

    ImGui::SliderFloat("Opacity", &global.transparency, 0.0, 1.0);

    ImGui::End();
}