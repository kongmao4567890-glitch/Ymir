#include "debug_widgets.hpp"

#include <app/actions.hpp>
#include <app/events/emu_event_factory.hpp>
#include <app/input/input_utils.hpp>

#include <imgui.h>

namespace app::ui::widgets {

void DebugWarning(SharedContext &ctx) {
    const bool debugTracing = ctx.saturn.IsDebugTracingEnabled();
    if (!debugTracing) {
        ImGui::TextColored(ctx.colors.warn, "调试跟踪已禁用。部分功能将无法使用。");
        ImGui::SameLine();
        if (ImGui::SmallButton(fmt::format("启用 ({})##debug_tracing",
                                           input::ToShortcut(ctx.inputContext, actions::dbg::ToggleDebugTrace))
                                   .c_str())) {
            ctx.EnqueueEvent(events::emu::SetDebugTrace(true));
        }
    }
}

} // namespace app::ui::widgets