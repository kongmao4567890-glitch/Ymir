#include "gui_settings_view.hpp"

#include <app/ui/widgets/common_widgets.hpp>

#include <ymir/sys/clocks.hpp>

namespace app::ui {

GUISettingsView::GUISettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void GUISettingsView::Display() {
    auto &settings = GetSettings();
    auto &guiSettings = settings.gui;

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("常规");
    ImGui::PopFont();

    MakeDirty(ImGui::Checkbox("在窗口标题栏显示游戏名称", &guiSettings.showGameNameOnTitleBar));
    MakeDirty(
        ImGui::Checkbox("在窗口标题栏显示性能指标", &guiSettings.showPerformanceOnTitleBar));
    widgets::ExplanationTooltip("在标题栏显示模拟速度、VDP2、VDP1 和界面帧率",
                                m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("界面缩放");
    ImGui::PopFont();

    // Round scale to steps of 25% and clamp to 100%-200% range
    bool overrideUIScale = guiSettings.overrideUIScale;
    double uiScale = overrideUIScale ? guiSettings.uiScale.Get() : m_context.displayScale;
    uiScale = std::round(uiScale / 0.25) * 0.25;
    uiScale = std::clamp(uiScale, 1.00, 2.00);

    if (MakeDirty(ImGui::Checkbox(fmt::format("覆盖界面缩放（当前：{:.0f}%）", uiScale * 100.0).c_str(),
                                  &overrideUIScale))) {
        guiSettings.overrideUIScale = overrideUIScale;
        // Use current DPI setting when enabling the override
        if (overrideUIScale) {
            guiSettings.uiScale = uiScale;
        }
    }

    ImGui::Indent();
    if (!overrideUIScale) {
        ImGui::BeginDisabled();
    }
    if (MakeDirty(ImGui::RadioButton("100%##ui_scale", uiScale == 1.0))) {
        guiSettings.uiScale = 1.00;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("125%##ui_scale", uiScale == 1.25))) {
        guiSettings.uiScale = 1.25;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("150%##ui_scale", uiScale == 1.50))) {
        guiSettings.uiScale = 1.50;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("175%##ui_scale", uiScale == 1.75))) {
        guiSettings.uiScale = 1.75;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("200%##ui_scale", uiScale == 2.00))) {
        guiSettings.uiScale = 2.00;
    }
    if (!overrideUIScale) {
        ImGui::EndDisabled();
    }
    ImGui::Unindent();

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("行为");
    ImGui::PopFont();

    MakeDirty(ImGui::Checkbox("记住窗口几何属性", &guiSettings.rememberWindowGeometry));
    widgets::ExplanationTooltip(
        "启用后，下次启动应用程序时将恢复当前窗口的位置和大小。",
        m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("屏幕显示");
    ImGui::PopFont();

    MakeDirty(ImGui::Checkbox("显示消息", &guiSettings.showMessages));
    widgets::ExplanationTooltip(
        "启用后，通知消息将显示在窗口的左上角。",
        m_context.displayScale);

    const bool isPAL = settings.system.videoStandard.Get() == ymir::core::config::sys::VideoStandard::PAL;

    MakeDirty(ImGui::Checkbox("显示帧率", &guiSettings.showFrameRateOSD));
    widgets::ExplanationTooltip(
        fmt::format(
            "显示一个包含 VDP2、VDP1 和界面帧率以及目标模拟速度的小型覆盖层。\n"
            "\n"
            "- VDP2 帧率表示模拟器的整体速度。如果在 100% 速度下模拟时低于 60 或 50 fps（分别对应 NTSC 或 PAL），"
            "说明你的系统跟不上。（当前视频制式设置为 {0}，因此目标帧率为 {1:.0f}。）\n"
            "- VDP1 帧率可能因游戏而异 - VDP2 帧率的一半或三分之一是常见的比例。它可能为零，甚至高于 {1:.0f} fps。\n"
            "- 界面帧率表示用户界面刷新的速度。它应该与显示器的刷新率或 VDP2 目标帧率匹配。如果启用了视频同步，"
            "界面更新会被调整以确保在具有可变刷新率显示器的强劲机器上获得流畅体验。\n"
            "- 速度表示（可调整的）目标模拟速度。100% 为实时速度。",
            (isPAL ? "PAL" : "NTSC"), (isPAL ? ymir::sys::kPALFrameRate : ymir::sys::kNTSCFrameRate))
            .c_str(),
        m_context.displayScale);
    ImGui::Indent();
    auto frameRateOSDOption = [&](const char *name, Settings::GUI::FrameRateOSDPosition value) {
        if (MakeDirty(ImGui::RadioButton(name, guiSettings.frameRateOSDPosition == value))) {
            guiSettings.frameRateOSDPosition = value;
        }
    };
    frameRateOSDOption("左上##fps_osd", Settings::GUI::FrameRateOSDPosition::TopLeft);
    ImGui::SameLine();
    frameRateOSDOption("右上##fps_osd", Settings::GUI::FrameRateOSDPosition::TopRight);
    ImGui::SameLine();
    frameRateOSDOption("左下##fps_osd", Settings::GUI::FrameRateOSDPosition::BottomLeft);
    ImGui::SameLine();
    frameRateOSDOption("右下##fps_osd", Settings::GUI::FrameRateOSDPosition::BottomRight);
    ImGui::Unindent();

    MakeDirty(
        ImGui::Checkbox("为修改的速度显示速度指示器", &guiSettings.showSpeedIndicatorForAllSpeeds));
    widgets::ExplanationTooltip(
        "启用后，对于 100% 以外的任何模拟速度都会显示速度指示器。\n"
        "禁用时，仅在加速运行时显示速度指示器。\n"
        "暂停或回退时始终显示速度指示器。",
        m_context.displayScale);
}

} // namespace app::ui
