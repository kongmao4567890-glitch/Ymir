#include "sh2_divu_registers_view.hpp"

#include <ymir/hw/sh2/sh2.hpp>

#include <app/events/emu_debug_event_factory.hpp>

using namespace ymir;

namespace app::ui {

SH2DivisionUnitRegistersView::SH2DivisionUnitRegistersView(SharedContext &context, ymir::sh2::SH2 &sh2)
    : m_context(context)
    , m_sh2(sh2) {}

void SH2DivisionUnitRegistersView::Display() {
    auto &probe = m_sh2.GetProbe();
    auto &divu = probe.DIVU();
    auto &intc = probe.INTC();

    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    if (ImGui::BeginTable("divu_regs", 4, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();

        auto drawReg = [&](uint32 &value, const char *name, const char *tooltip) {
            ImGui::BeginGroup();
            ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 8);
            ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
            const bool changed = ImGui::InputScalar(fmt::format("##{}", name).c_str(), ImGuiDataType_U32, &value,
                                                    nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(name);
            ImGui::EndGroup();
            ImGui::SetItemTooltip("%s", tooltip);
            return changed;
        };

        if (ImGui::TableNextColumn()) {
            drawReg(divu.DVDNTH, "DVDNTH", "64 位被除数高位");
            drawReg(divu.DVDNTUH, "DVDNTUH", "64 位被除数高位 (影子副本)");
        }

        if (ImGui::TableNextColumn()) {
            drawReg(divu.DVDNTL, "DVDNTL", "64 位被除数低位");
            drawReg(divu.DVDNTUL, "DVDNTUL", "64 位被除数低位 (影子副本)");
        }

        if (ImGui::TableNextColumn()) {
            drawReg(divu.DVDNT, "DVDNT", "32 位被除数");
            drawReg(divu.DVSR, "DVSR", "除数");
        }

        if (ImGui::TableNextColumn()) {
            uint32 dvcr = divu.DVCR.Read();
            if (drawReg(dvcr, "DVCR", "除法控制寄存器")) {
                divu.DVCR.Write(dvcr);
            }
            ImGui::Checkbox("OVF", &divu.DVCR.OVF);
            ImGui::SetItemTooltip("溢出标志");
            ImGui::SameLine();
            ImGui::Checkbox("OVFIE", &divu.DVCR.OVFIE);
            ImGui::SetItemTooltip("溢出中断启用");
        }

        ImGui::EndTable();
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("中断:");

    ImGui::SameLine();

    ImGui::BeginGroup();
    uint8 vector = intc.GetVector(sh2::InterruptSource::DIVU_OVFI);
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 2);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    if (ImGui::InputScalar("##vcrdiv", ImGuiDataType_U8, &vector, nullptr, nullptr, "%02X",
                           ImGuiInputTextFlags_CharsHexadecimal)) {
        intc.SetVector(sh2::InterruptSource::DIVU_OVFI, vector);
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextUnformatted("VCRDIV");
    ImGui::EndGroup();
    ImGui::SetItemTooltip("除法单元中断向量");

    ImGui::SameLine();

    uint8 level = intc.GetLevel(sh2::InterruptSource::DIVU_OVFI);
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(ImGui::GetStyle().FramePadding.x * 2 + hexCharWidth * 1);
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    if (ImGui::InputScalar("##ipra_divuipn", ImGuiDataType_U8, &level, nullptr, nullptr, "%X",
                           ImGuiInputTextFlags_CharsHexadecimal)) {
        intc.SetLevel(sh2::InterruptSource::DIVU_OVFI, std::min<uint8>(level, 0xF));
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextUnformatted("IPRA.DIVUIP3-0");
    ImGui::EndGroup();
    ImGui::SetItemTooltip("除法单元中断级别");

    const bool master = m_sh2.IsMaster();

    ImGui::SameLine(0, 15.0f);
    ImGui::TextUnformatted("计算:");
    ImGui::SameLine();
    if (ImGui::Button("32x32")) {
        m_context.EnqueueEvent(events::emu::debug::ExecuteSH2Division(master, false));
    }
    ImGui::SameLine();
    if (ImGui::Button("64x32")) {
        m_context.EnqueueEvent(events::emu::debug::ExecuteSH2Division(master, true));
    }
}

} // namespace app::ui
