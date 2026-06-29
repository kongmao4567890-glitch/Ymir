#include "analog_pad_config_view.hpp"

namespace app::ui {

AnalogPadConfigView::AnalogPadConfigView(SharedContext &context)
    : SettingsViewBase(context)
    , m_inputCaptureWidget(context, m_unboundActionsWidget)
    , m_unboundActionsWidget(context) {}

void AnalogPadConfigView::Display(Settings::Input::Port::AnalogPad &controllerSettings, uint32 portIndex) {
    auto &settings = GetSettings();
    auto &binds = controllerSettings.binds;

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("模式：");
    ImGui::SameLine();
    if (ImGui::RadioButton("模拟", m_context.analogPadInputs[portIndex].analogMode)) {
        m_context.analogPadInputs[portIndex].analogMode = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("数字", !m_context.analogPadInputs[portIndex].analogMode)) {
        m_context.analogPadInputs[portIndex].analogMode = false;
    }

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
        ImGui::TableSetupColumn("按钮", ImGuiTableColumnFlags_WidthFixed, 85.0f * m_context.displayScale);
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
                    m_inputCaptureWidget.DrawInputBindButton(bind, i, &m_context.analogPadInputs[portIndex]);
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
        drawRow(binds.analogStick);
        drawRow(binds.analogL);
        drawRow(binds.analogR);
        drawRow(binds.switchMode);

        m_inputCaptureWidget.DrawCapturePopup();

        ImGui::EndTable();
    }
}

} // namespace app::ui
