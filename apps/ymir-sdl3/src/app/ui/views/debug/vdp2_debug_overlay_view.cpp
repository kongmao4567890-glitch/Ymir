#include "vdp2_debug_overlay_view.hpp"

#include <ymir/hw/vdp/vdp.hpp>

#include <app/events/emu_debug_event_factory.hpp>

#include <imgui.h>

using namespace ymir;

namespace app::ui {

VDP2DebugOverlayView::VDP2DebugOverlayView(SharedContext &context, vdp::VDP &vdp)
    : m_context(context)
    , m_vdp(vdp) {}

void VDP2DebugOverlayView::Display() {
    auto &overlay = m_vdp.vdp2DebugRenderOptions.overlay;
    using OverlayType = vdp::config::VDP2DebugRender::Overlay::Type;

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    auto overlayName = [](OverlayType type) {
        switch (type) {
        case OverlayType::None: return "无覆盖层";
        case OverlayType::SingleLayer: return "单图层";
        case OverlayType::LayerStack: return "图层堆栈";
        case OverlayType::PriorityStack: return "优先级堆栈";
        case OverlayType::Windows: return "窗口";
        case OverlayType::RotParams: return "RBG0 旋转参数";
        case OverlayType::ColorCalc: return "颜色计算";
        case OverlayType::ColorGradation: return "颜色渐变";
        case OverlayType::Shadow: return "阴影";
        default: return "无效";
        }
    };

    auto colorPicker = [&](const char *name, vdp::Color888 &color) {
        std::array<float, 3> colorFloat{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f};
        if (ImGui::ColorEdit3(name, colorFloat.data())) {
            color.r = colorFloat[0] * 255.0f;
            color.g = colorFloat[1] * 255.0f;
            color.b = colorFloat[2] * 255.0f;
        }
    };

    ImGui::BeginGroup();

    // TODO: enqueue events
    // TODO: persist parameters
    ImGui::Checkbox("启用调试渲染", &m_vdp.vdp2DebugRenderOptions.overlay.enable);

    if (!m_vdp.vdp2DebugRenderOptions.overlay.enable) {
        ImGui::BeginDisabled();
    }

    ImGui::SeparatorText("覆盖层");
    if (ImGui::BeginCombo("类型##overlay", overlayName(overlay.type), ImGuiComboFlags_HeightLargest)) {
        auto option = [&](OverlayType type) {
            if (ImGui::Selectable(overlayName(type), overlay.type == type)) {
                overlay.type = type;
            }
        };
        option(OverlayType::None);
        option(OverlayType::SingleLayer);
        option(OverlayType::LayerStack);
        option(OverlayType::PriorityStack);
        option(OverlayType::Windows);
        option(OverlayType::RotParams);
        option(OverlayType::ColorCalc);
        option(OverlayType::ColorGradation);
        option(OverlayType::Shadow);
        ImGui::EndCombo();
    }

    if (overlay.type == OverlayType::None) {
        ImGui::BeginDisabled();
    }
    static constexpr uint8 kMinAlpha = 0;
    static constexpr uint8 kMaxAlpha = 255;
    ImGui::SliderScalar("Alpha##vdp2_overlay", ImGuiDataType_U8, &overlay.alpha, &kMinAlpha, &kMaxAlpha, nullptr,
                        ImGuiSliderFlags_AlwaysClamp);
    if (overlay.type == OverlayType::None) {
        ImGui::EndDisabled();
    }

    switch (overlay.type) {
    case OverlayType::SingleLayer: //
    {
        auto layerName = [](uint8 index) {
            switch (index) {
            case 0: return "精灵";
            case 1: return "RBG0";
            case 2: return "NBG0/RBG1";
            case 3: return "NBG1/EXBG";
            case 4: return "NBG2";
            case 5: return "NBG3";
            case 6: return "背景屏幕";
            case 7: return "行颜色屏幕";
            case 8: return "透明网格精灵";
            case 9: return "颜色渐变屏幕";
            default: return "无效";
            }
        };
        if (ImGui::BeginCombo("图层##single", layerName(overlay.singleLayerIndex), ImGuiComboFlags_HeightLargest)) {
            for (uint32 i = 0; i <= 9; ++i) {
                const std::string label = fmt::format("{}##single_layer", layerName(i));
                if (ImGui::Selectable(label.c_str(), overlay.singleLayerIndex == i)) {
                    overlay.singleLayerIndex = i;
                }
            }
            ImGui::EndCombo();
        }
        break;
    }
    case OverlayType::LayerStack: //
    {
        static constexpr uint8 kMinStackIndex = 0;
        static constexpr uint8 kMaxStackIndex = 2;
        ImGui::SliderScalar("图层级别##vdp2_overlay_layer_stack", ImGuiDataType_U8, &overlay.layerStackIndex,
                            &kMinStackIndex, &kMaxStackIndex, nullptr, ImGuiSliderFlags_AlwaysClamp);
        colorPicker("精灵##layer_stack", overlay.layerColors[0]);
        colorPicker("RBG0##layer_stack", overlay.layerColors[1]);
        colorPicker("NBG0/RBG1##layer_stack", overlay.layerColors[2]);
        colorPicker("NBG1/EXBG##layer_stack", overlay.layerColors[3]);
        colorPicker("NBG2##layer_stack", overlay.layerColors[4]);
        colorPicker("NBG3##layer_stack", overlay.layerColors[5]);
        colorPicker("背景##layer_stack", overlay.layerColors[6]);
        // colorPicker("Line color", overlay.layerColors[7]);
        break;
    }
    case OverlayType::PriorityStack: //
    {
        static constexpr uint8 kMinStackIndex = 0;
        static constexpr uint8 kMaxStackIndex = 2;
        ImGui::SliderScalar("图层级别##vdp2_overlay_priority_stack", ImGuiDataType_U8, &overlay.priorityStackIndex,
                            &kMinStackIndex, &kMaxStackIndex, nullptr, ImGuiSliderFlags_AlwaysClamp);
        for (uint32 i = 0; i < 8; ++i) {
            colorPicker(fmt::format("{}##layer_stack", i).c_str(), overlay.priorityColors[i]);
        }
        break;
    }
    case OverlayType::Windows: //
    {
        auto windowLayerName = [](uint8 index) {
            switch (index) {
            case 0: return "精灵";
            case 1: return "RBG0";
            case 2: return "NBG0/RBG1";
            case 3: return "NBG1/EXBG";
            case 4: return "NBG2";
            case 5: return "NBG3";
            case 6: return "旋转参数";
            case 7: return "颜色计算";
            default: return "自定义";
            }
        };

        if (ImGui::BeginCombo("图层##window", windowLayerName(overlay.windowLayerIndex),
                              ImGuiComboFlags_HeightLargest)) {
            for (uint32 i = 0; i <= 8; ++i) {
                const std::string label = fmt::format("{}##window_layer", windowLayerName(i));
                if (ImGui::Selectable(label.c_str(), overlay.windowLayerIndex == i)) {
                    overlay.windowLayerIndex = i;
                }
            }
            ImGui::EndCombo();
        }
        // TODO: show window set state for other layers (read-only)
        if (overlay.windowLayerIndex > 7) {
            if (ImGui::BeginTable("custom_window", 2, ImGuiTableFlags_SizingFixedFit)) {
                for (uint32 i = 0; i < 3; ++i) {
                    ImGui::PushID(i);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted("W0");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("启用", &overlay.customWindowSet.enabled[i]);
                    ImGui::SameLine();
                    ImGui::Checkbox("反转", &overlay.customWindowSet.inverted[i]);
                    if (i < 2) {
                        ImGui::SameLine();
                        ImGui::Checkbox("行表:", &overlay.customLineWindowTableEnable[i]);
                        ImGui::SameLine();
                        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                        ImGui::SetNextItemWidth(5 * hexCharWidth + 2 * paddingWidth);
                        ImGui::InputScalar("##linetbl_addr", ImGuiDataType_U32,
                                           &overlay.customLineWindowTableAddress[i], nullptr, nullptr, "%05X");
                        ImGui::PopFont();
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("组合:");
            ImGui::SameLine();
            if (ImGui::RadioButton("OR", overlay.customWindowSet.logic == vdp::WindowLogic::Or)) {
                overlay.customWindowSet.logic = vdp::WindowLogic::Or;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("AND", overlay.customWindowSet.logic == vdp::WindowLogic::And)) {
                overlay.customWindowSet.logic = vdp::WindowLogic::And;
            }
        }
        colorPicker("内部##window", overlay.windowInsideColor);
        colorPicker("外部##window", overlay.windowOutsideColor);
        break;
    }
    case OverlayType::RotParams: //
        colorPicker("A##rotparam", overlay.rotParamAColor);
        colorPicker("B##rotparam", overlay.rotParamBColor);
        break;
    case OverlayType::ColorCalc: //
    {
        static constexpr uint8 kMinLayerStackIndex = 0;
        static constexpr uint8 kMaxLayerStackIndex = 1;
        ImGui::SliderScalar("图层级别##vdp2_overlay_color_calc", ImGuiDataType_U8, &overlay.colorCalcStackIndex,
                            &kMinLayerStackIndex, &kMaxLayerStackIndex, nullptr, ImGuiSliderFlags_AlwaysClamp);
        colorPicker("已禁用##color_grad", overlay.colorCalcDisableColor);
        colorPicker("已启用##color_grad", overlay.colorCalcEnableColor);
        break;
    }
    case OverlayType::ColorGradation: //
    {
        static constexpr uint8 kMinLayerStackIndex = 0;
        static constexpr uint8 kMaxLayerStackIndex = 1;
        ImGui::SliderScalar("图层级别##vdp2_overlay_color_grad", ImGuiDataType_U8, &overlay.colorGradStackIndex,
                            &kMinLayerStackIndex, &kMaxLayerStackIndex, nullptr, ImGuiSliderFlags_AlwaysClamp);
        colorPicker("已禁用##color_grad", overlay.colorGradDisableColor);
        colorPicker("已启用##color_grad", overlay.colorGradEnableColor);
        break;
    }
    case OverlayType::Shadow: //
    {
        colorPicker("已禁用##shadow", overlay.shadowDisableColor);
        colorPicker("已启用##shadow", overlay.shadowEnableColor);
        break;
    }
    default: break;
    }
    ImGui::Unindent();

    if (!m_vdp.vdp2DebugRenderOptions.overlay.enable) {
        ImGui::EndDisabled();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
