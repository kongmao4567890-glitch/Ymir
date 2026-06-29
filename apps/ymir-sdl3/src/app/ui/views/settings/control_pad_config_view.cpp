#include "control_pad_config_view.hpp"

namespace app::ui {

ControlPadConfigView::ControlPadConfigView(SharedContext &context)
    : SettingsViewBase(context)
    , m_inputCaptureWidget(context, m_unboundActionsWidget)
    , m_unboundActionsWidget(context) {}

void ControlPadConfigView::Display(Settings::Input::Port::ControlPad &controllerSettings, uint32 portIndex) {
    auto &settings = GetSettings();
    auto &binds = controllerSettings.binds;

    if (ImGui::Button("恢复默认")) {
        m_unboundActionsWidget.Capture(settings.ResetBinds(binds, true));
        MakeDirty();
    }
    ImGui::SameLine();
    if (ImGui::Button("全部清除")) {
        m_unboundActionsWidget.Capture(settings.ResetBinds(binds, false));
        MakeDirty();
    }

    ImGui::TextUnformatted("左键点击按钮以分配快捷键。右键点击以清除。");
    m_unboundActionsWidget.Display();
    if (ImGui::BeginTable("hotkeys", 1 + input::kNumBindsPerInput,
                          ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("按钮", ImGuiTableColumnFlags_WidthFixed, 70.0f * m_context.displayScale);
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
                    m_inputCaptureWidget.DrawInputBindButton(bind, i, &m_context.controlPadInputs[portIndex]);
                }
            }
        };

        drawRow(binds.a);
        drawRow(binds.b);
        drawRow(binds.c);
        drawRow(binds.x);
        drawRow(binds.y);
        drawRow(binds.z);
        drawRow(binds.l);
        drawRow(binds.r);
        drawRow(binds.start);
        drawRow(binds.up);
        drawRow(binds.down);
        drawRow(binds.left);
        drawRow(binds.right);
        drawRow(binds.dpad);

        m_inputCaptureWidget.DrawCapturePopup();

        ImGui::EndTable();
    }
}

} // namespace app::ui
