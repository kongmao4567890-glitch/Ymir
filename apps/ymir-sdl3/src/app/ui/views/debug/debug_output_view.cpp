#include "debug_output_view.hpp"

#include <app/events/gui_event_factory.hpp>

#include <util/sdl_file_dialog.hpp>

#include <misc/cpp/imgui_stdlib.h>

namespace app::ui {

DebugOutputView::DebugOutputView(SharedContext &context)
    : m_context(context)
    , m_tracer(context.tracers.SCU) {}

void DebugOutputView::Display() {
    if (ImGui::Button("清除##debug_output")) {
        m_tracer.ClearDebugMessages();
    }
    ImGui::SameLine();
    if (ImGui::Button("保存到文件...##debug_output")) {
        m_context.EnqueueEvent(events::gui::SaveFile(
            {.dialogTitle = "导出调试输出",
             .defaultPath = m_context.profile.GetPath(ProfilePath::Dumps) / "debug.txt",
             .filters = {{.name = "文本文件 (*.txt)", .filters = "txt"}},
             .userdata = this,
             .callback = util::WrapSingleSelectionCallback<&DebugOutputView::ProcessExportDebugOutput,
                                                           util::NoopCancelFileDialogCallback,
                                                           &DebugOutputView::ProcessExportError>}));
    }

    if (ImGui::BeginChild("##scu_debug_output")) {
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.small);
        std::string fullBuffer{};
        const size_t count = m_tracer.debugMessages.Count();
        for (size_t i = 0; i < count; i++) {
            fullBuffer.append(m_tracer.debugMessages.Read(i)).append("\n");
        }
        fullBuffer.append(m_tracer.GetDebugMessageBuffer());
        ImGui::InputTextMultiline("##debug_output", &fullBuffer, ImGui::GetContentRegionAvail(),
                                  ImGuiInputTextFlags_ReadOnly);
        ImGui::PopFont();
    }
    ImGui::EndChild();
}

void DebugOutputView::ProcessExportDebugOutput(void *userdata, std::filesystem::path file, int filter) {
    static_cast<DebugOutputView *>(userdata)->ExportDebugOutput(file);
}

void DebugOutputView::ProcessCancelExport(void *userdata, int filter) {}

void DebugOutputView::ProcessExportError(void *userdata, const char *errorMessage, int filter) {
    static_cast<DebugOutputView *>(userdata)->ShowErrorDialog(errorMessage);
}

void DebugOutputView::ExportDebugOutput(std::filesystem::path file) {
    std::filesystem::create_directories(file.parent_path());

    std::ofstream out{file};
    const size_t count = m_tracer.debugMessages.Count();
    for (size_t i = 0; i < count; i++) {
        out << m_tracer.debugMessages.Read(i) << "\n";
    }
    out << m_tracer.GetDebugMessageBuffer();
}

void DebugOutputView::ShowErrorDialog(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(message));
}

} // namespace app::ui
