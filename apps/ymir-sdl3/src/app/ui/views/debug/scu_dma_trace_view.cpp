#include "scu_dma_trace_view.hpp"

namespace app::ui {

SCUDMATraceView::SCUDMATraceView(SharedContext &context)
    : m_context(context)
    , m_tracer(context.tracers.SCU) {}

void SCUDMATraceView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    ImGui::BeginGroup();

    ImGui::Checkbox("启用", &m_tracer.traceDMA);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("你还必须在 调试 > 启用跟踪 (F11) 中启用跟踪");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::Button("清除")) {
        m_tracer.ClearDMATransfers();
    }

    if (ImGui::BeginTable("dma_trace", 6,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("通道", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("间接", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 7);
        ImGui::TableSetupColumn("源", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 10);
        ImGui::TableSetupColumn("目标", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 10);
        ImGui::TableSetupColumn("长度", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 7);
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        const size_t count = m_tracer.dmaTransfers.Count();
        for (size_t i = 0; i < count; i++) {
            auto *sort = ImGui::TableGetSortSpecs();
            bool reverse = false;
            if (sort != nullptr && sort->SpecsCount == 1) {
                reverse = sort->Specs[0].SortDirection == ImGuiSortDirection_Descending;
            }

            auto trace = reverse ? m_tracer.dmaTransfers.ReadReverse(i) : m_tracer.dmaTransfers.Read(i);

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%u", trace.counter);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%u", trace.channel);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                if (trace.indirect) {
                    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                    ImGui::Text("%07X", trace.indirectAddr);
                    ImGui::PopFont();
                } else {
                    ImGui::TextUnformatted("否");
                }
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%07X", trace.srcAddr);
                ImGui::PopFont();
                ImGui::SameLine();
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.small);
                ImGui::TextDisabled("+%d", trace.srcAddrInc);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%07X", trace.dstAddr);
                ImGui::PopFont();
                ImGui::SameLine();
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.small);
                ImGui::TextDisabled("+%d", trace.dstAddrInc);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%X", trace.xferCount);
                ImGui::PopFont();
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
