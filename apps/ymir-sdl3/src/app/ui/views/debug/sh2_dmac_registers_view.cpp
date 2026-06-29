#include "sh2_dmac_registers_view.hpp"

#include <ymir/hw/sh2/sh2.hpp>

using namespace ymir;

namespace app::ui {

SH2DMAControllerRegistersView::SH2DMAControllerRegistersView(SharedContext &context, ymir::sh2::SH2 &sh2)
    : m_context(context)
    , m_sh2(sh2) {}

void SH2DMAControllerRegistersView::Display() {
    auto &probe = m_sh2.GetProbe();
    auto &dmaor = probe.DMAOR();
    auto &intc = probe.INTC();

    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    uint32 DMAOR = dmaor.Read();

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 8);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    ImGui::InputScalar("##dmaor", ImGuiDataType_U32, &DMAOR, nullptr, nullptr, "%08X",
                       ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("DMAOR");
    ImGui::EndGroup();
    ImGui::SetItemTooltip("DMA 操作寄存器");

    ImGui::SameLine();
    ImGui::Checkbox("PR", &dmaor.PR);
    ImGui::SetItemTooltip("优先级模式");

    ImGui::SameLine();
    ImGui::Checkbox("AE", &dmaor.AE);
    ImGui::SetItemTooltip("地址错误标志");

    ImGui::SameLine();
    ImGui::Checkbox("NMIF", &dmaor.NMIF);
    ImGui::SetItemTooltip("NMI 标志");

    ImGui::SameLine();
    ImGui::Checkbox("DME", &dmaor.DME);
    ImGui::SetItemTooltip("DMA 主使能");

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("中断：");
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        uint8 vector = intc.GetVector(sh2::InterruptSource::DMAC0_XferEnd);
        ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 2);
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        if (ImGui::InputScalar("##vcrdma0", ImGuiDataType_U8, &vector, nullptr, nullptr, "%02X",
                               ImGuiInputTextFlags_CharsHexadecimal)) {
            intc.SetVector(sh2::InterruptSource::DMAC0_XferEnd, vector);
        }
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::TextUnformatted("VCRDMA0");
        ImGui::EndGroup();
        ImGui::SetItemTooltip("DMA 通道 0 传输结束向量");
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        uint8 vector = intc.GetVector(sh2::InterruptSource::DMAC1_XferEnd);
        ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 2);
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        if (ImGui::InputScalar("##vcrdma1", ImGuiDataType_U8, &vector, nullptr, nullptr, "%02X",
                               ImGuiInputTextFlags_CharsHexadecimal)) {
            intc.SetVector(sh2::InterruptSource::DMAC1_XferEnd, vector);
        }
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::TextUnformatted("VCRDMA1");
        ImGui::EndGroup();
        ImGui::SetItemTooltip("DMA 通道 1 传输结束向量");
    }
    ImGui::SameLine();
    {
        uint8 level = intc.GetLevel(sh2::InterruptSource::DMAC0_XferEnd);
        ImGui::BeginGroup();
        ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 1);
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        if (ImGui::InputScalar("##ipra_dmacipn", ImGuiDataType_U8, &level, nullptr, nullptr, "%X",
                               ImGuiInputTextFlags_CharsHexadecimal)) {
            intc.SetLevel(sh2::InterruptSource::DMAC0_XferEnd, std::min<uint8>(level, 0xF));
        }
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::TextUnformatted("IPRA.DMACIP3-0");
        ImGui::EndGroup();
        ImGui::SetItemTooltip("DMA 控制器中断级别");
    }
}

} // namespace app::ui
