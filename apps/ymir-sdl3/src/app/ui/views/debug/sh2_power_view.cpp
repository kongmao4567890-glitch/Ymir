#include "sh2_power_view.hpp"

#include <ymir/hw/sh2/sh2.hpp>

#include <app/ui/widgets/common_widgets.hpp>

using namespace ymir;

namespace app::ui {

SH2PowerView::SH2PowerView(SharedContext &context, ymir::sh2::SH2 &sh2)
    : m_context(context)
    , m_sh2(sh2) {}

void SH2PowerView::Display() {
    auto &probe = m_sh2.GetProbe();
    auto &sbycr = probe.SBYCR();

    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 2);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    ImGui::InputScalar("##sbycr", ImGuiDataType_U8, &sbycr.u8, nullptr, nullptr, "%02X",
                       ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("SBYCR");
    ImGui::EndGroup();
    ImGui::SetItemTooltip("待机控制寄存器");

    bool value;

    value = sbycr.MSTP0;
    if (ImGui::Checkbox("停止并重置 SCI", &value)) {
        sbycr.MSTP0 = value;
    }
    widgets::ExplanationTooltip("串行通信接口", m_context.displayScale);

    value = sbycr.MSTP1;
    if (ImGui::Checkbox("停止并重置 FRT", &value)) {
        sbycr.MSTP1 = value;
    }
    widgets::ExplanationTooltip("自由运行定时器", m_context.displayScale);

    value = sbycr.MSTP2;
    if (ImGui::Checkbox("停止并重置 DIVU", &value)) {
        sbycr.MSTP2 = value;
    }
    widgets::ExplanationTooltip("除法单元", m_context.displayScale);

    value = sbycr.MSTP3;
    if (ImGui::Checkbox("停止并重置 MULT", &value)) {
        sbycr.MSTP3 = value;
    }
    widgets::ExplanationTooltip("乘法单元", m_context.displayScale);

    value = sbycr.MSTP4;
    if (ImGui::Checkbox("停止并重置 DMAC", &value)) {
        sbycr.MSTP4 = value;
    }
    widgets::ExplanationTooltip("DMA 控制器", m_context.displayScale);

    value = sbycr.HIZ;
    if (ImGui::Checkbox("端口高阻态", &value)) {
        sbycr.HIZ = value;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("模式:");
    ImGui::SameLine();
    if (ImGui::RadioButton("休眠", sbycr.SBY == 0)) {
        sbycr.SBY = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("待机", sbycr.SBY == 1)) {
        sbycr.SBY = 1;
    }

    ImGui::Separator();

    const bool master = m_sh2.IsMaster();
    if (!master) {
        bool slaveSH2Enabled = m_context.saturn.IsSlaveSH2Enabled();
        if (ImGui::Checkbox("已启用", &slaveSH2Enabled)) {
            m_context.saturn.SetSlaveSH2Enabled(slaveSH2Enabled);
        }
    }

    if (!m_context.saturn.IsDebugTracingEnabled()) {
        ImGui::BeginDisabled();
    }
    bool suspended = m_sh2.IsCPUSuspended();
    if (ImGui::Checkbox("已挂起", &suspended)) {
        m_sh2.SetCPUSuspended(suspended);
    }
    widgets::ExplanationTooltip("在调试模式下禁用 CPU。", m_context.displayScale);
    if (!m_context.saturn.IsDebugTracingEnabled()) {
        ImGui::EndDisabled();
    }

    bool asleep = probe.GetSleepState();
    if (ImGui::Checkbox("休眠", &asleep)) {
        probe.SetSleepState(asleep);
    }
    widgets::ExplanationTooltip("CPU 是否因执行 SLEEP 指令而处于待机或休眠模式。",
                                m_context.displayScale);
}

} // namespace app::ui
