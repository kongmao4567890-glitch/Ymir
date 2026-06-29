#include "virtua_gun_config_view.hpp"

namespace app::ui {

VirtuaGunConfigView::VirtuaGunConfigView(SharedContext &context)
    : SettingsViewBase(context)
    , m_inputCaptureWidget(context, m_unboundActionsWidget)
    , m_unboundActionsWidget(context) {}

void VirtuaGunConfigView::Display(Settings::Input::Port::VirtuaGun &controllerSettings, uint32 portIndex) {
    auto &settings = GetSettings();
    auto &binds = controllerSettings.binds;
    auto &xhair = controllerSettings.crosshair;

    using namespace app::config_defaults::input::virtua_gun;

    // -------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("行为");
    ImGui::PopFont();

    if (ImGui::Button("恢复默认##speed")) {
        controllerSettings.speed = kDefaultSpeed;
        controllerSettings.speedBoostFactor = kDefaultSpeedBoostFactor;
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

    // -------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("十字准星");
    ImGui::PopFont();

    ImGui::BeginGroup();
    {
        const float scale = m_context.displayScale;
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 size{150.0f * scale, 150.0f * scale};
        const ImVec2 end{pos.x + size.x, pos.y + size.y};
        static constexpr ImU32 kBorderColor = 0xE0F5D4C6;
        const ImU32 bgColor = ImGui::ColorConvertFloat4ToU32(
            ImVec4{m_crosshairPreviewBGColor[0], m_crosshairPreviewBGColor[1], m_crosshairPreviewBGColor[2], 1.0f});

        ImDrawList *drawList = ImGui::GetWindowDrawList();

        const widgets::CrosshairParams params{
            .color = {xhair.color[0], xhair.color[1], xhair.color[2], xhair.color[3]},
            .radius = xhair.radius,
            .thickness = xhair.thickness,
            .rotation = xhair.rotation,

            .strokeColor = {xhair.strokeColor[0], xhair.strokeColor[1], xhair.strokeColor[2], xhair.strokeColor[3]},
            .strokeThickness = xhair.strokeThickness,

            .displayScale = scale,
        };

        drawList->AddRectFilled(pos, end, bgColor);

        drawList->PushClipRect(pos, end, true);
        widgets::Crosshair(drawList, params, {pos.x + size.x * 0.5f, pos.y + size.y * 0.5f});
        drawList->PopClipRect();

        drawList->AddRect(pos, end, kBorderColor, 0.0f, ImDrawFlags_None, 1.0f * scale);

        ImGui::Dummy(size);

        ImGui::ColorEdit3("背景", m_crosshairPreviewBGColor.data(), ImGuiColorEditFlags_NoInputs);
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    {
        using namespace crosshair;
        if (ImGui::BeginTable("crosshair_params", 2, ImGuiTableColumnFlags_WidthStretch)) {
            ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("颜色");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            MakeDirty(ImGui::ColorEdit4("##color", xhair.color.data(), ImGuiColorEditFlags_AlphaBar));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("半径");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            MakeDirty(ImGui::SliderFloat("##radius", &xhair.radius, kMinRadius, kMaxRadius, "%.1f",
                                         ImGuiSliderFlags_AlwaysClamp));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("粗细");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            float thickness = xhair.thickness * 100.0f;
            if (MakeDirty(ImGui::SliderFloat("##thickness", &thickness, kMinThickness * 100.0f, kMaxThickness * 100.0f,
                                             "%.1f%%", ImGuiSliderFlags_AlwaysClamp))) {
                xhair.thickness = thickness / 100.0f;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("旋转");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            MakeDirty(ImGui::SliderFloat("##rotation", &xhair.rotation, 0.0f, 90.0f, "%.1f\u00B0",
                                         ImGuiSliderFlags_AlwaysClamp));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("描边颜色");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            MakeDirty(ImGui::ColorEdit4("##stroke_color", xhair.strokeColor.data(), ImGuiColorEditFlags_AlphaBar));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("描边粗细");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            float strokeThickness = xhair.strokeThickness * 100.0f;
            if (MakeDirty(ImGui::SliderFloat("##stroke_thickness", &strokeThickness, kMinStrokeThickness * 100.0f,
                                             kMaxStrokeThickness * 100.0f, "%.1f%%", ImGuiSliderFlags_AlwaysClamp))) {
                xhair.strokeThickness = strokeThickness / 100.0f;
            }

            ImGui::EndTable();
        }

        if (ImGui::Button("恢复默认##crosshair")) {
            xhair.color = kDefaultColor[portIndex];
            xhair.radius = kDefaultRadius[portIndex];
            xhair.thickness = kDefaultThickness[portIndex];
            xhair.rotation = kDefaultRotation[portIndex];
            xhair.strokeColor = kDefaultStrokeColor[portIndex];
            xhair.strokeThickness = kDefaultStrokeThickness[portIndex];
            MakeDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("随机##crosshair")) {

            xhair.color = {m_randomDist(m_randomEngine), m_randomDist(m_randomEngine), m_randomDist(m_randomEngine),
                           m_randomDist(m_randomEngine) * 0.4f + 0.6f};
            xhair.radius = m_randomDist(m_randomEngine) * (kMaxRadius - kMinRadius) + kMinRadius;
            xhair.thickness = m_randomDist(m_randomEngine) * (kMaxThickness - kMinThickness) + kMinThickness;
            xhair.rotation = m_randomDist(m_randomEngine) * 90.0f;
            xhair.strokeColor = {m_randomDist(m_randomEngine), m_randomDist(m_randomEngine),
                                 m_randomDist(m_randomEngine), m_randomDist(m_randomEngine) * 0.4f + 0.6f};
            xhair.strokeThickness =
                m_randomDist(m_randomEngine) * (kMaxStrokeThickness - kMinStrokeThickness) + kMinStrokeThickness;
            MakeDirty();
        }
    }
    ImGui::EndGroup();

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
                    m_inputCaptureWidget.DrawInputBindButton(bind, i, &m_context.virtuaGunInputs[portIndex]);
                }
            }
        };

        drawRow(binds.start);
        drawRow(binds.trigger);
        drawRow(binds.reload);
        drawRow(binds.up);
        drawRow(binds.down);
        drawRow(binds.left);
        drawRow(binds.right);
        drawRow(binds.move);
        drawRow(binds.recenter);
        drawRow(binds.speedBoost);
        drawRow(binds.speedToggle);

        m_inputCaptureWidget.DrawCapturePopup();

        ImGui::EndTable();
    }

    // -------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("鼠标绑定");
    ImGui::PopFont();

    // TODO: configurable mouse inputs
    ImGui::TextUnformatted("鼠标输入按以下方式绑定：");

    if (ImGui::BeginTable("mouse_hotkeys", 2, ImGuiTableFlags_SizingFixedFit)) {
        auto drawRow = [&](const char *name, const char *button) {
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::TextUnformatted(name);
            }
            if (ImGui::TableNextColumn()) {
                ImGui::TextUnformatted(button);
            }
        };

        drawRow("扳机", "左键");
        drawRow("装弹", "右键");
        drawRow("Start", "中键");

        ImGui::EndTable();
    }
}

} // namespace app::ui
