#include "vdp2_sprite_layer_params_view.hpp"

#include <app/ui/utils/debug/vdp_window_set_printer.hpp>

#include <ymir/hw/vdp/vdp.hpp>

#include <imgui.h>

using namespace ymir;

namespace app::ui {

VDP2SpriteLayerParamsView::VDP2SpriteLayerParamsView(SharedContext &context, vdp::VDP &vdp)
    : m_context(context)
    , m_vdp(vdp) {}

void VDP2SpriteLayerParamsView::Display() {
    auto &probe = m_vdp.GetProbe();
    const auto &regs2 = probe.GetVDP2Regs();
    const auto &state2 = probe.GetVDP2State();

    auto printYesNo = [&](bool value) {
        if (value) {
            ImGui::TextUnformatted("是");
        } else {
            ImGui::TextUnformatted("否");
        }
    };

    if (ImGui::BeginTable("sprite", 2, ImGuiTableFlags_SizingFixedFit)) {
        const auto &spriteParams = regs2.spriteParams;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("格式");

        ImGui::TableNextColumn();
        ImGui::Text("类型 %u, ", spriteParams.type);
        ImGui::SameLine(0, 0);
        if (spriteParams.mixedFormat) {
            ImGui::TextUnformatted("调色板/RGB");
        } else {
            ImGui::TextUnformatted("仅调色板");
        }
        if (spriteParams.lineColorScreenEnable) {
            ImGui::SameLine(0, 0);
            ImGui::TextUnformatted(", LNCL 插入");
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("颜色计算");
        ImGui::TableNextColumn();
        if (spriteParams.colorCalcEnable) {
            switch (spriteParams.colorCalcCond) {
            case vdp::SpriteColorCalculationCondition::PriorityLessThanOrEqual:
                ImGui::Text("优先级 <= %u", spriteParams.colorCalcValue);
                break;
            case vdp::SpriteColorCalculationCondition::PriorityEqual:
                ImGui::Text("优先级 == %u", spriteParams.colorCalcValue);
                break;
            case vdp::SpriteColorCalculationCondition::PriorityGreaterThanOrEqual:
                ImGui::Text("优先级 >= %u", spriteParams.colorCalcValue);
                break;
            case vdp::SpriteColorCalculationCondition::MsbEqualsOne: ImGui::TextUnformatted("颜色 MSB == 1"); break;
            }
            ImGui::SameLine(0, 0);
            ImGui::Text(
                ", 比率: %u %u %u %u %u %u %u %u", spriteParams.colorCalcRatios[0], spriteParams.colorCalcRatios[1],
                spriteParams.colorCalcRatios[2], spriteParams.colorCalcRatios[3], spriteParams.colorCalcRatios[4],
                spriteParams.colorCalcRatios[5], spriteParams.colorCalcRatios[6], spriteParams.colorCalcRatios[7]);
        } else {
            ImGui::TextUnformatted("否");
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("优先级");
        ImGui::TableNextColumn();
        for (uint32 i = 0; i < 8; ++i) {
            if (i > 0) {
                ImGui::SameLine();
            }
            ImGui::Text("%u", spriteParams.priorities[i]);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("窗口");
        ImGui::TableNextColumn();
        const auto &windowSet = spriteParams.windowSet;
        WindowSetPrinter printer{windowSet.logic};
        printer.AppendWindow("0", windowSet.enabled[0], windowSet.inverted[0]);
        printer.AppendWindow("1", windowSet.enabled[1], windowSet.inverted[1]);
        printer.AppendWindow("S", spriteParams.spriteWindowEnabled, spriteParams.spriteWindowInverted);
        ImGui::Text("%s", printer.ToString().c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("精灵窗口");
        ImGui::TableNextColumn();
        printYesNo(spriteParams.useSpriteWindow);

        ImGui::EndTable();
    }
}

} // namespace app::ui
