#include "sh2_dmac_channel_view.hpp"

using namespace ymir;

namespace app::ui {

SH2DMAControllerChannelView::SH2DMAControllerChannelView(SharedContext &context, ymir::sh2::DMAChannel &channel,
                                                         int index)
    : m_context(context)
    , m_channel(channel)
    , m_index(index) {}

void SH2DMAControllerChannelView::Display() {
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    auto calcSpacing = [&](const char *label) {
        const float len0 = ImGui::CalcTextSize(fmt::format("{}0", label).c_str()).x;
        const float len1 = ImGui::CalcTextSize(fmt::format("{}1", label).c_str()).x;
        const float currLen = m_index == 0 ? len0 : len1;
        const float maxLen = std::max(len0, len1);
        return maxLen - currLen + ImGui::GetStyle().ItemSpacing.x;
    };

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 8);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    ImGui::InputScalar(fmt::format("##sar{}", m_index).c_str(), ImGuiDataType_U32, &m_channel.srcAddress, nullptr,
                       nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("SAR%d", m_index);
    ImGui::EndGroup();
    ImGui::SetItemTooltip("DMA 源地址寄存器");

    ImGui::SameLine(0, calcSpacing("SAR"));

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 8);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    ImGui::InputScalar(fmt::format("##dar{}", m_index).c_str(), ImGuiDataType_U32, &m_channel.dstAddress, nullptr,
                       nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("DAR%d", m_index);
    ImGui::EndGroup();
    ImGui::SetItemTooltip("DMA 目标地址寄存器");

    ImGui::SameLine(0, calcSpacing("DAR"));

    uint32 xferCount = m_channel.xferCount;
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 6);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    if (ImGui::InputScalar(fmt::format("##tcr{}", m_index).c_str(), ImGuiDataType_U32, &xferCount, nullptr, nullptr,
                           "%06X", ImGuiInputTextFlags_CharsHexadecimal)) {
        m_channel.xferCount = xferCount;
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("TCR%d", m_index);
    ImGui::EndGroup();
    ImGui::SetItemTooltip("DMA 传输计数寄存器");

    ImGui::SameLine(0, calcSpacing("TCR"));

    uint32 CHCR = m_channel.ReadCHCR();
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 8);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    if (ImGui::InputScalar(fmt::format("##chcr{}", m_index).c_str(), ImGuiDataType_U32, &CHCR, nullptr, nullptr, "%08X",
                           ImGuiInputTextFlags_CharsHexadecimal)) {
        m_channel.WriteCHCR<true>(CHCR);
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("CHCR%d", m_index);
    ImGui::EndGroup();
    ImGui::SetItemTooltip("DMA 通道控制寄存器");

    ImGui::SameLine(0, calcSpacing("CHCR"));

    uint8 DRCR = m_channel.ReadDRCR();
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 2);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    if (ImGui::InputScalar(fmt::format("##drcr{}", m_index).c_str(), ImGuiDataType_U8, &DRCR, nullptr, nullptr, "%02X",
                           ImGuiInputTextFlags_CharsHexadecimal)) {
        m_channel.WriteDRCR(DRCR);
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("DRCR%d", m_index);
    ImGui::EndGroup();
    ImGui::SetItemTooltip("DMA 请求/响应选择控制寄存器");

    ImGui::Separator();

    ImGui::Checkbox(fmt::format("已启用##{}", m_index).c_str(), &m_channel.xferEnabled);
    ImGui::SameLine();
    ImGui::Checkbox(fmt::format("中断启用##{}", m_index).c_str(), &m_channel.irqEnable);
    ImGui::SameLine();
    ImGui::Checkbox(fmt::format("自动请求模式##{}", m_index).c_str(), &m_channel.autoRequest);
    ImGui::SameLine();
    ImGui::Checkbox(fmt::format("传输结束##{}", m_index).c_str(), &m_channel.xferEnded);

    if (ImGui::BeginTable("chcr_values", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("源地址模式");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("固定##src", m_channel.srcMode == sh2::DMATransferIncrementMode::Fixed)) {
                m_channel.srcMode = sh2::DMATransferIncrementMode::Fixed;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("递增##src", m_channel.srcMode == sh2::DMATransferIncrementMode::Increment)) {
                m_channel.srcMode = sh2::DMATransferIncrementMode::Increment;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("递减##src", m_channel.srcMode == sh2::DMATransferIncrementMode::Decrement)) {
                m_channel.srcMode = sh2::DMATransferIncrementMode::Decrement;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("目标地址模式");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("固定##dst", m_channel.dstMode == sh2::DMATransferIncrementMode::Fixed)) {
                m_channel.dstMode = sh2::DMATransferIncrementMode::Fixed;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("递增##dst", m_channel.dstMode == sh2::DMATransferIncrementMode::Increment)) {
                m_channel.dstMode = sh2::DMATransferIncrementMode::Increment;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("递减##dst", m_channel.dstMode == sh2::DMATransferIncrementMode::Decrement)) {
                m_channel.dstMode = sh2::DMATransferIncrementMode::Decrement;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("传输单元大小");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("8 位##xfer_size", m_channel.xferSize == sh2::DMATransferSize::Byte)) {
                m_channel.xferSize = sh2::DMATransferSize::Byte;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("16 位##xfer_size", m_channel.xferSize == sh2::DMATransferSize::Word)) {
                m_channel.xferSize = sh2::DMATransferSize::Word;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("32 位##xfer_size", m_channel.xferSize == sh2::DMATransferSize::Longword)) {
                m_channel.xferSize = sh2::DMATransferSize::Longword;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("4x32 位##xfer_size", m_channel.xferSize == sh2::DMATransferSize::QuadLongword)) {
                m_channel.xferSize = sh2::DMATransferSize::QuadLongword;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("传输总线模式");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("周期窃取##xfer_bus_mode",
                                   m_channel.xferBusMode == sh2::DMATransferBusMode::CycleSteal)) {
                m_channel.xferBusMode = sh2::DMATransferBusMode::CycleSteal;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("突发##xfer_bus_mode", m_channel.xferBusMode == sh2::DMATransferBusMode::Burst)) {
                m_channel.xferBusMode = sh2::DMATransferBusMode::Burst;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("传输地址模式");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("双地址##xfer_addr_mode",
                                   m_channel.xferAddressMode == sh2::DMATransferAddressMode::Dual)) {
                m_channel.xferAddressMode = sh2::DMATransferAddressMode::Dual;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("单地址##xfer_addr_mode",
                                   m_channel.xferAddressMode == sh2::DMATransferAddressMode::Single)) {
                m_channel.xferAddressMode = sh2::DMATransferAddressMode::Single;
            }
        }

        const bool dualAddrMode = m_channel.xferAddressMode == sh2::DMATransferAddressMode::Dual;
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(dualAddrMode ? "应答模式" : "传输模式");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton(dualAddrMode ? "读周期期间##ack_xfer_mode"
                                                : "内存 -> 设备##ack_xfer_mode",
                                   !m_channel.ackXferMode)) {
                m_channel.ackXferMode = false;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton(dualAddrMode ? "写周期期间##ack_xfer_mode"
                                                : "设备 -> 内存##ack_xfer_mode",
                                   m_channel.ackXferMode)) {
                m_channel.ackXferMode = true;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("DACK 电平");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("高电平有效##dack", !m_channel.ackLevel)) {
                m_channel.ackLevel = false;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("低电平有效##dack", m_channel.ackLevel)) {
                m_channel.ackLevel = true;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("请求/响应选择");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("DREQ##res", m_channel.resSelect == sh2::DMAResourceSelect::DREQ)) {
                m_channel.resSelect = sh2::DMAResourceSelect::DREQ;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("RXI##res", m_channel.resSelect == sh2::DMAResourceSelect::RXI)) {
                m_channel.resSelect = sh2::DMAResourceSelect::RXI;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("TXI##res", m_channel.resSelect == sh2::DMAResourceSelect::TXI)) {
                m_channel.resSelect = sh2::DMAResourceSelect::TXI;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("DREQ 选择");
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("电平检测##dreq", m_channel.dreqSelect == sh2::SignalDetectionMode::Level)) {
                m_channel.dreqSelect = sh2::SignalDetectionMode::Level;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("边沿检测##dreq", m_channel.dreqSelect == sh2::SignalDetectionMode::Edge)) {
                m_channel.dreqSelect = sh2::SignalDetectionMode::Edge;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("DREQ 电平");
        }
        if (ImGui::TableNextColumn()) {
            const bool levelDetect = m_channel.dreqSelect == sh2::SignalDetectionMode::Level;
            if (ImGui::RadioButton(levelDetect ? "低电平##dreq" : "下降沿##dreq", !m_channel.dreqLevel)) {
                m_channel.dreqLevel = false;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton(levelDetect ? "高电平##dreq" : "上升沿##dreq", m_channel.dreqLevel)) {
                m_channel.dreqLevel = true;
            }
        }

        ImGui::EndTable();
    }
}

} // namespace app::ui
