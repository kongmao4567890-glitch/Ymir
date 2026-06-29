#include "shuttle_mouse_config_view.hpp"

namespace app::ui {

ShuttleMouseConfigView::ShuttleMouseConfigView(SharedContext &context)
    : SettingsViewBase(context)
    , m_inputCaptureWidget(context, m_unboundActionsWidget)
    , m_unboundActionsWidget(context) {}

void ShuttleMouseConfigView::Display(Settings::Input::Port::ShuttleMouse &controllerSettings, uint32 portIndex) {
    auto &settings = GetSettings();
    auto &binds = controllerSettings.binds;

    using namespace app::config_defaults::input::shuttle_mouse;

    // -------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("行为");
    ImGui::PopFont();

    if (ImGui::Button("恢复默认##speed")) {
        controllerSettings.speed = kDefaultSpeed;
        controllerSettings.speedBoostFactor = kDefaultSpeedBoostFactor;
        controllerSettings.sensitivity = kDefaultSensitivity;
        MakeDirty();
    }

    float speed = controllerSettings.speed.Get();
    if (MakeDirty(ImGui::SliderFloat("速度", &speed, kMinSpeed, kMaxSpeed, "%.0f", ImGuiSliderFlags_AlwaysClamp))) {
        controllerSettings.speed = speed;
    }
    float speedBoostFactor = controllerSettings.speedBoostFactor.Get() * 100.0f;
    if (MakeDirty(ImGui::SliderFloat("加速倍数", &speedBoostFactor, kMinSpeedBoostFactor * 100.0f,
                                     kMaxSpeedBoostFactor * 100.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp))) {
        controllerSettings.speedBoostFactor = speedBoostFactor / 100.0f;
    }
    float sensitivity = controllerSettings.sensitivity.Get();
    if (MakeDirty(ImGui::SliderFloat("鼠标灵敏度", &sensitivity, kMinSensitivity, kMaxSensitivity, "%.2fx",
                                     ImGuiSliderFlags_AlwaysClamp))) {
        controllerSettings.sensitivity = sensitivity;
    }

    // -------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("绑定");
    ImGui::PopFont();

    if (ImGui::Button("恢复默认##binds")) {
        m_unboundActionsWidget.Capture(settings.ResetBinds(binds, true));
        MakeDirty();
    }
    ImGui::SameLine();
    if (ImGui::Button("全部清除")) {
        m_unboundActionsWidget.Capture(settings.ResetBinds(binds, false));
        MakeDirty();
    }

    ImGui::TextUnformatted("左键、中键和右键鼠标按钮正常映射。");
    ImGui::TextUnformatted("Start 绑定到鼠标按钮 4 和 5。");
    ImGui::TextUnformatted("左键点击按钮以分配快捷键。右键点击以清除。");
    m_unboundActionsWidget.Display();
    if (ImGui::BeginTable("hotkeys", 1 + input::kNumBindsPerInput, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("按钮", ImGuiTableColumnFlags_WidthFixed, 90.0f * m_context.displayScale);
        for (size_t i = 0; i < input::kNumBindsPerInput; i++) {
            ImGui::TableSetupColumn(fmt::format("快捷键 {}", i + 1).c_str(), ImGuiTableColumnFlags_WidthStretch, 1.0f);
        }
        ImGui::TableHeadersRow();

        auto drawRow = [&](input::InputBind &bind) {
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(bind.action.name);
            }
            for (uint32 i = 0; i < input::kNumBindsPerInput; i++) {
                if (ImGui::TableNextColumn()) {
                    m_inputCaptureWidget.DrawInputBindButton(bind, i, &m_context.shuttleMouseInputs[portIndex]);
                }
            }
        };

        drawRow(binds.start);
        drawRow(binds.left);
        drawRow(binds.middle);
        drawRow(binds.right);
        drawRow(binds.moveUp);
        drawRow(binds.moveDown);
        drawRow(binds.moveLeft);
        drawRow(binds.moveRight);
        drawRow(binds.move);
        drawRow(binds.speedBoost);
        drawRow(binds.speedToggle);

        m_inputCaptureWidget.DrawCapturePopup();

        ImGui::EndTable();
    }
}

} // namespace app::ui
