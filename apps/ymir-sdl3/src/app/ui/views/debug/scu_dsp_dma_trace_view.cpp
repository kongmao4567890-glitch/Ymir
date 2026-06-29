#include "scu_dsp_dma_trace_view.hpp"

namespace app::ui {

SCUDSPDMATraceView::SCUDSPDMATraceView(SharedContext &context)
    : m_context(context)
    , m_tracer(context.tracers.SCU) {}

void SCUDSPDMATraceView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    ImGui::BeginGroup();

    ImGui::Checkbox("启用", &m_tracer.traceDSPDMA);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("你还必须在 调试 > 启用跟踪 (F11) 中启用跟踪");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::Button("清除")) {
        m_tracer.ClearDSPDMATransfers();
    }

    if (ImGui::BeginTable("dsp_dma_trace", 4,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("源", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 10);
        ImGui::TableSetupColumn("目标", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 10);
        ImGui::TableSetupColumn("长度", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 3);
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        const size_t count = m_tracer.dspDmaTransfers.Count();
        for (size_t i = 0; i < count; i++) {
            auto *sort = ImGui::TableGetSortSpecs();
            bool reverse = false;
            if (sort != nullptr && sort->SpecsCount == 1) {
                reverse = sort->Specs[0].SortDirection == ImGuiSortDirection_Descending;
            }

            auto trace = reverse ? m_tracer.dspDmaTransfers.ReadReverse(i) : m_tracer.dspDmaTransfers.Read(i);

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%u", trace.counter);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                if (trace.toD0) {
                    switch (trace.addrDSP) {
                    case 0: ImGui::TextUnformatted("数据 RAM 0"); break;
                    case 1: ImGui::TextUnformatted("数据 RAM 1"); break;
                    case 2: ImGui::TextUnformatted("数据 RAM 2"); break;
                    case 3: ImGui::TextUnformatted("数据 RAM 3"); break;
                    default: ImGui::Text("无效 (%u)", trace.addrDSP); break;
                    }
                } else {
                    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                    ImGui::Text("%07X", trace.addrD0);
                    ImGui::PopFont();
                    ImGui::SameLine();
                    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.small);
                    if (trace.addrInc > 0) {
                        if (trace.hold) {
                            ImGui::TextDisabled("(+%d)", trace.addrInc);
                        } else {
                            ImGui::TextDisabled("+%d", trace.addrInc);
                        }
                    } else if (trace.addrInc < 0) {
                        if (trace.hold) {
                            ImGui::TextDisabled("(-%d)", -trace.addrInc);
                        } else {
                            ImGui::TextDisabled("-%d", -trace.addrInc);
                        }
                    }
                    ImGui::PopFont();
                }
            }
            if (ImGui::TableNextColumn()) {
                if (trace.toD0) {
                    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                    ImGui::Text("%07X", trace.addrD0);
                    ImGui::PopFont();
                    ImGui::SameLine();
                    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.small);
                    if (trace.addrInc > 0) {
                        ImGui::TextDisabled("+%d", trace.addrInc);
                    } else if (trace.addrInc < 0) {
                        ImGui::TextDisabled("-%d", -trace.addrInc);
                    }
                    ImGui::PopFont();
                } else {
                    switch (trace.addrDSP) {
                    case 0: ImGui::TextUnformatted("数据 RAM 0"); break;
                    case 1: ImGui::TextUnformatted("数据 RAM 1"); break;
                    case 2: ImGui::TextUnformatted("数据 RAM 2"); break;
                    case 3: ImGui::TextUnformatted("数据 RAM 3"); break;
                    case 4: ImGui::TextUnformatted("程序 RAM"); break;
                    default: ImGui::Text("无效 (%u)", trace.addrDSP); break;
                    }
                }
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%X", trace.count == 0 ? 0x100 : trace.count);
                ImGui::PopFont();
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
