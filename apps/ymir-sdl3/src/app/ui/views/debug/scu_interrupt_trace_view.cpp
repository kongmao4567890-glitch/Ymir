#include "scu_interrupt_trace_view.hpp"

namespace app::ui {

SCUInterruptTraceView::SCUInterruptTraceView(SharedContext &context)
    : m_context(context)
    , m_tracer(context.tracers.SCU) {}

void SCUInterruptTraceView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    constexpr const char *kNames[] = {"VDP2 VBlank 进入",     "VDP2 VBlank 退出",     "VDP2 HBlank 进入",
                                      "SCU 定时器 0",         "SCU 定时器 1",         "SCU DSP 结束",
                                      "SCSP 声音请求",  "SMPC 系统管理器", "SMPC 手柄中断",
                                      "SCU Level 2 DMA 结束", "SCU Level 1 DMA 结束", "SCU Level 0 DMA 结束",
                                      "SCU DMA 非法",     "VDP1 精灵绘制结束"};

    ImGui::BeginGroup();

    ImGui::Checkbox("启用", &m_tracer.traceInterrupts);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("你还必须在 调试 > 启用跟踪 (F11) 中启用跟踪");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::Button("清除")) {
        m_tracer.ClearInterrupts();
    }

    if (ImGui::BeginTable("intr_trace", 4,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("级别", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * 1);
        ImGui::TableSetupColumn("应答", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + ImGui::CalcTextSize("yes").x);
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
                ImGui::Text("%X", trace.level);
            }
            if (ImGui::TableNextColumn()) {
                if (trace.acknowledged) {
                    ImGui::TextUnformatted("是");
                }
            }
            if (ImGui::TableNextColumn()) {
                if (trace.index < std::size(kNames)) {
                    ImGui::Text("%s", kNames[trace.index]);
                } else if (trace.index >= 16) {
                    ImGui::Text("外部 %X", trace.index - 16);
                } else {
                    ImGui::Text("无效 (%X)", trace.index);
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
