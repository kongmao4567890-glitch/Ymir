#include "sh2_interrupt_trace_view.hpp"

using namespace ymir;

namespace app::ui {

SH2InterruptTraceView::SH2InterruptTraceView(SharedContext &context, SH2Tracer &tracer)
    : m_context(context)
    , m_tracer(tracer) {}

void SH2InterruptTraceView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    ImGui::BeginGroup();

    ImGui::Checkbox("启用", &m_tracer.traceInterrupts);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("你还必须在 调试 > 启用跟踪 (F11) 中启用跟踪");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::Button("清除##trace")) {
        m_tracer.interrupts.Clear();
        m_tracer.ResetInterruptCounter();
    }

    if (ImGui::BeginTable("intr_trace", 5,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 8);
        ImGui::TableSetupColumn("向量", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 2);
        ImGui::TableSetupColumn("级别", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 2);
        ImGui::TableSetupColumn("源", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        const size_t count = m_tracer.interrupts.Count();
        for (size_t i = 0; i < count; i++) {
            auto *sort = ImGui::TableGetSortSpecs();
            bool reverse = false;
            if (sort != nullptr && sort->SpecsCount == 1) {
                reverse = sort->Specs[0].SortDirection == ImGuiSortDirection_Descending;
            }

            auto trace = reverse ? m_tracer.interrupts.ReadReverse(i) : m_tracer.interrupts.Read(i);

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%u", trace.counter);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%08X", trace.pc);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%02X", trace.vecNum);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%X", trace.level);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::TextUnformatted(sh2::GetInterruptSourceName(trace.source).data());
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
