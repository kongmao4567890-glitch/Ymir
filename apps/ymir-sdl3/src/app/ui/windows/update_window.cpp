#include "update_window.hpp"

#include <ymir/version.hpp>

namespace app::ui {

UpdateWindow::UpdateWindow(SharedContext &context)
    : WindowBase(context) {

    m_windowConfig.name = "发现新版本";
    m_windowConfig.flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
}

void UpdateWindow::PrepareWindow() {
    // Close window if no updates are actually available
    {
        std::unique_lock lock{m_context.locks.targetUpdate};
        if (!m_context.targetUpdate) {
            Open = false;
            return;
        }
    }

    auto *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + vp->Size.y * 0.5f), ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));
}

void UpdateWindow::DrawContents() {
    ImGui::TextUnformatted("发现 Ymir 新版本。");
    ImGui::TextUnformatted("当前版本：" Ymir_VERSION);
    ImGui::TextUnformatted("新版本：");
    ImGui::SameLine(0, 0);
    {
        std::unique_lock lock{m_context.locks.targetUpdate};
        if (m_context.targetUpdate) {
            auto &info = m_context.targetUpdate->info;
            ImGui::TextLinkOpenURL(info.version.to_string().c_str(), info.downloadURL.c_str());
            ImGui::TextLinkOpenURL("发行说明", info.releaseNotesURL.c_str());
        } else {
            ImGui::TextUnformatted("正在获取...");
        }
    }

    ImGui::Separator();
    if (ImGui::Button("关闭")) {
        Open = false;
    }
}

} // namespace app::ui
