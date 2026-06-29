#include "update_onboarding_window.hpp"

#include <app/settings.hpp>

#include <app/ui/widgets/common_widgets.hpp>

#include <app/events/gui_event_factory.hpp>

#include <util/os_features.hpp>

#include <ymir/version.hpp>

#include <fstream>

namespace app::ui {

UpdateOnboardingWindow::UpdateOnboardingWindow(SharedContext &context)
    : WindowBase(context) {

    m_windowConfig.name = "自动检查更新";
    m_windowConfig.flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
}

void UpdateOnboardingWindow::PrepareWindow() {
    auto *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + vp->Size.y * 0.5f), ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));
}

void UpdateOnboardingWindow::DrawContents() {
    ImGui::PushTextWrapPos(450.0f * m_context.displayScale);

    ImGui::TextUnformatted("Ymir 可以在启动时自动检查新版本。");
    ImGui::TextUnformatted("这需要网络连接，并将访问 github.com 来检查新版本。");
    ImGui::TextUnformatted("请在下方做出选择：");
    ImGui::Checkbox("启动时检查更新", &m_checkForUpdates);
    widgets::ExplanationTooltip(
        "启用后，Ymir 会在每次启动时检查更新，并在有新版本时通知你。\n"
        "接受后，如果启用了此选项，Ymir 将立即检查更新。",
        m_context.displayScale);
    ImGui::Checkbox("更新到 nightly 版本", &m_includeNightlyBuilds);
    widgets::ExplanationTooltip(
        "Ymir 检查更新时，也会考虑 nightly 版本。\n"
        "Nightly 版本包含最新的功能和错误修复，但属于开发中版本，可能存在错误",
        m_context.displayScale);

    ImGui::NewLine();
    ImGui::TextUnformatted("点击“接受”应用这些设置，或点击“稍后决定”关闭此窗口。\n"
                           "如果选择稍后决定，此弹窗将在下次启动时再次出现。");

    ImGui::Separator();
    if (ImGui::Button("接受")) {
        const auto updatesPath = m_context.profile.GetPath(ProfilePath::PersistentState) / "updates";
        const auto onboardedPath = updatesPath / ".onboarded";
        std::filesystem::create_directories(updatesPath);
        std::ofstream{onboardedPath};

        util::os::SetFileHidden(onboardedPath, true);

        auto &settings = m_context.serviceLocator.GetRequired<Settings>();
        settings.general.checkForUpdates = m_checkForUpdates;
        settings.general.includeNightlyBuilds = m_includeNightlyBuilds;
        if (m_checkForUpdates) {
            m_context.EnqueueEvent(events::gui::CheckForUpdates());
        }
        Open = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("稍后决定")) {
        Open = false;
    }

    ImGui::PopTextWrapPos();
}

} // namespace app::ui
