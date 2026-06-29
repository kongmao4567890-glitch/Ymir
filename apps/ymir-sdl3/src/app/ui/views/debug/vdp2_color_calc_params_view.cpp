#include "vdp2_color_calc_params_view.hpp"

#include <ymir/hw/vdp/vdp.hpp>

#include <imgui.h>

using namespace ymir;

namespace app::ui {

VDP2ColorCalcParamsView::VDP2ColorCalcParamsView(SharedContext &context, vdp::VDP &vdp)
    : m_context(context)
    , m_vdp(vdp) {}

void VDP2ColorCalcParamsView::Display() {
    auto &probe = m_vdp.GetProbe();
    const auto &regs2 = probe.GetVDP2Regs();

    if (regs2.colorCalcParams.colorGradEnable) {
        ImGui::TextUnformatted("颜色渐变: 已启用在 ");
        ImGui::SameLine(0, 0);
        switch (regs2.colorCalcParams.colorGradScreen) {
        case vdp::ColorGradScreen::Sprite: ImGui::TextUnformatted("精灵"); break;
        case vdp::ColorGradScreen::NBG0_RBG1:
            if (regs2.bgEnabled[5]) {
                ImGui::TextUnformatted("RBG1");
            } else {
                ImGui::TextUnformatted("NBG0");
            }
            break;
        case vdp::ColorGradScreen::NBG1_EXBG:
            if (regs2.EXTEN.EXBGEN) {
                ImGui::TextUnformatted("EXBG");
            } else {
                ImGui::TextUnformatted("NBG1");
            }
            break;
        case vdp::ColorGradScreen::NBG2: ImGui::TextUnformatted("NBG2"); break;
        case vdp::ColorGradScreen::NBG3: ImGui::TextUnformatted("NBG3"); break;
        case vdp::ColorGradScreen::RBG0: ImGui::TextUnformatted("RBG0"); break;
        case vdp::ColorGradScreen::Invalid3: ImGui::TextUnformatted("(无效) [3]"); break;
        case vdp::ColorGradScreen::Invalid7: ImGui::TextUnformatted("(无效) [7]"); break;
        }
    } else {
        ImGui::TextUnformatted("颜色渐变: 已禁用");
    }

    if (regs2.colorCalcParams.extendedColorCalcEnable) {
        ImGui::TextUnformatted("使用扩展颜色计算");
        if (regs2.colorCalcParams.colorGradEnable) {
            ImGui::SameLine();
            ImGui::TextUnformatted("(已忽略；颜色渐变优先)");
        }
    } else {
        ImGui::TextUnformatted("使用标准颜色计算");
    }

    if (regs2.colorCalcParams.useSecondScreenRatio) {
        ImGui::TextUnformatted("使用第二屏幕比率");
    } else {
        ImGui::TextUnformatted("使用顶层屏幕比率");
    }

    if (regs2.colorCalcParams.useAdditiveBlend) {
        ImGui::TextUnformatted("混合模式: 加法");
    } else {
        ImGui::TextUnformatted("混合模式: Alpha (颜色比率)");
    }
}

} // namespace app::ui
