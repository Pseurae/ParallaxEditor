#include "UI/OptionsPane.h"
#include <filesystem>
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"

void OptionsPane(void)
{
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SliderFloat("Opacity", &global.transparency, 0.0, 1.0);
    ImGui::End();
}