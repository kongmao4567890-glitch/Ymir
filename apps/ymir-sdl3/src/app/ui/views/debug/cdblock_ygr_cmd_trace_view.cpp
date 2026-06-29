#include "cdblock_ygr_cmd_trace_view.hpp"

#include <app/settings.hpp>

#include <ymir/util/bit_ops.hpp>

namespace app::ui {

static ImVec4 MakeColorFromU8(uint8 value) {
    ImVec4 color;
    color.w = 1.0f;
    value = bit::reverse(value);
    const float hue = static_cast<float>(value) / static_cast<float>(0xFF);
    ImGui::ColorConvertHSVtoRGB(hue, 0.63f, 1.00f, color.x, color.y, color.z);
    return color;
}

YGRCommandTraceView::YGRCommandTraceView(SharedContext &context)
    : m_context(context)
    , m_tracer(context.tracers.YGR) {}

void YGRCommandTraceView::Display() {
    const auto &settings = m_context.serviceLocator.GetRequired<Settings>();

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    ImGui::BeginGroup();

    ImGui::Checkbox("启用", &m_tracer.traceCommands);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("你还必须在 调试 > 启用跟踪 (F11) 中启用跟踪");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::Button("清除")) {
        m_tracer.ClearCommands();
    }
    if (!settings.cdblock.useLLE) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextColored(
            m_context.colors.notice,
            "CD Block LLE 已禁用。命令将改为跟踪到 CD Block 命令跟踪窗口。");
        ImGui::PopTextWrapPos();
    }

    if (ImGui::BeginTable("cdblock_cmd_trace", 3,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("请求", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * (4 + 1 + 4 + 1 + 4 + 1 + 4));
        ImGui::TableSetupColumn("响应", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * (4 + 1 + 4 + 1 + 4 + 1 + 4));
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        const size_t count = m_tracer.commands.Count();
        for (size_t i = 0; i < count; i++) {
            auto *sort = ImGui::TableGetSortSpecs();
            bool reverse = false;
            if (sort != nullptr && sort->SpecsCount == 1) {
                reverse = sort->Specs[0].SortDirection == ImGuiSortDirection_Descending;
            }

            auto trace = reverse ? m_tracer.commands.ReadReverse(i) : m_tracer.commands.Read(i);

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::Text("%u", trace.index);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                if (trace.reqValid) {
                    const uint8 cmd = trace.request[0] >> 8u;
                    ImGui::TextColored(MakeColorFromU8(cmd), "%04X %04X %04X %04X", trace.request[0], trace.request[1],
                                       trace.request[2], trace.request[3]);
                } else {
                    ImGui::TextUnformatted("---- ---- ---- ----");
                }
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                if (trace.resValid) {
                    const uint8 status = trace.response[0] >> 8u;
                    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                    ImGui::TextColored(MakeColorFromU8(status), "%04X %04X %04X %04X", trace.response[0],
                                       trace.response[1], trace.response[2], trace.response[3]);
                    ImGui::PopFont();
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
