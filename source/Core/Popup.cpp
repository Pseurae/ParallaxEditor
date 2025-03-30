#include "Popup.h"
#include <imgui.h>

void Popup::Draw(void)
{
    this->PreDrawContent();

    if (ImGui::BeginPopupModal(mInternalName.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
    {
        this->DrawContent();
        ImGui::EndPopup();
    }
}

void PopupManager::DrawAndUpdate(void)
{
    mPopups.remove_if([](const std::unique_ptr<Popup> &popup) {
        if (ImGui::IsPopupOpen(popup->GetInternalName().c_str()))
        {
            popup->Draw();
            return popup->ShouldClose();
        }
        else
        {
            ImGui::OpenPopup(popup->GetInternalName().c_str());
            return false;
        }
    });
}