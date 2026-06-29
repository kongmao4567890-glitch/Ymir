#include "sh2_debug_toolbar_view.hpp"

#include <ymir/hw/sh2/sh2.hpp>

#include <app/events/emu_event_factory.hpp>
#include <app/events/gui_event_factory.hpp>

#include <app/ui/fonts/IconsMaterialSymbols.h>

#include <app/ui/model/debug/sh2_debugger_model.hpp>

#include <app/ui/widgets/common_widgets.hpp>
#include <app/ui/widgets/debug_widgets.hpp>

#include <imgui.h>

#include <cstdint>

using namespace ymir;

namespace app::ui {

SH2DebugToolbarView::SH2DebugToolbarView(SharedContext &context, sh2::SH2 &sh2, SH2DebuggerModel &model)
    : m_context(context)
    , m_sh2(sh2)
    , m_model(model)
    , m_disasmDumpView(context, sh2) {}

void SH2DebugToolbarView::Display() {
    ImGui::BeginGroup();

    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();
    const float framePadding = ImGui::GetStyle().FramePadding.x;
    const float regFieldWidth = framePadding * 2 + hexCharWidth * 8;

    widgets::DebugWarning(m_context);

    const bool debugTracing = m_context.saturn.IsDebugTracingEnabled();
    const bool master = m_sh2.IsMaster();
    const bool enabled = master || m_context.saturn.IsSlaveSH2Enabled();
    const bool paused = m_context.paused;
    auto &probe = m_sh2.GetProbe();

    ImGui::BeginDisabled(!enabled);
    {
        if (ImGui::Button(ICON_MS_STEP)) {
            m_context.EnqueueEvent(master ? events::emu::StepMSH2() : events::emu::StepSSH2());
        }
        if (ImGui::BeginItemTooltip()) {
            ImGui::TextUnformatted("步进 (F7, S)");
            ImGui::EndTooltip();
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(paused);
        if (ImGui::Button(ICON_MS_PAUSE)) {
            m_context.EnqueueEvent(events::emu::SetPaused(true));
        }
        ImGui::EndDisabled();
        if (ImGui::BeginItemTooltip()) {
            ImGui::TextUnformatted("暂停 (Space, R)");
            ImGui::EndTooltip();
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(!paused);
        if (ImGui::Button(ICON_MS_PLAY_ARROW)) {
            m_context.EnqueueEvent(events::emu::SetPaused(false));
        }
        ImGui::EndDisabled();
        if (ImGui::BeginItemTooltip()) {
            ImGui::TextUnformatted("继续 (Space, R)");
            ImGui::EndTooltip();
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button(ICON_MS_REPLAY)) {
        m_context.EnqueueEvent(events::emu::HardReset());
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("硬重置 (Ctrl+R)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_MS_MASKED_TRANSITIONS)) {
        m_context.EnqueueEvent(events::gui::OpenSH2BreakpointsWindow(m_sh2.IsMaster()));
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("断点 (Ctrl+F9)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_MS_VISIBILITY)) {
        m_context.EnqueueEvent(events::gui::OpenSH2WatchpointsWindow(m_sh2.IsMaster()));
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("监视点 (Ctrl+Shift+F9)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_FILE_DOWNLOAD "##dump_disasm_range")) {
        m_disasmDumpView.OpenPopup();
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("转储反汇编范围 (Ctrl+D)");
        ImGui::EndTooltip();
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D)) {
        m_disasmDumpView.OpenPopup();
    }
    m_disasmDumpView.Display();

    if (!master) {
        ImGui::SameLine();
        bool slaveSH2Enabled = m_context.saturn.IsSlaveSH2Enabled();
        if (ImGui::Checkbox("已启用", &slaveSH2Enabled)) {
            m_context.saturn.SetSlaveSH2Enabled(slaveSH2Enabled);
        }
    }

    ImGui::SameLine();
    if (!debugTracing) {
        ImGui::BeginDisabled();
    }
    bool suspended = m_sh2.IsCPUSuspended();
    if (ImGui::Checkbox("已挂起", &suspended)) {
        m_sh2.SetCPUSuspended(suspended);
    }
    widgets::ExplanationTooltip("在调试模式下禁用 CPU。", m_context.displayScale);
    if (!debugTracing) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    bool asleep = probe.GetSleepState();
    if (ImGui::Checkbox("休眠", &asleep)) {
        probe.SetSleepState(asleep);
    }
    widgets::ExplanationTooltip("CPU 是否因执行 SLEEP 指令而处于待机或休眠模式。",
                                m_context.displayScale);

    auto doJump = [&] {
        // Align to even addresses
        m_model.jumpAddress = m_model.jumpAddress & ~1u;
        m_model.JumpTo(m_model.jumpAddress);
    };

    auto doJumpToPC = [&] {
        // Align to even addresses
        m_model.jumpAddress = probe.PC() & ~1u;
        m_model.JumpToPC();
    };

    // Input field to jump to address
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("跳转到:");

    ImGui::SameLine();
    if (ImGui::Button("PC##goto")) {
        doJumpToPC();
    }

    ImGui::SameLine();
    if (ImGui::Button("PR##goto")) {
        m_model.jumpAddress = probe.PR();
        doJump();
    }

    ImGui::SameLine();
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    ImGui::SetNextItemWidth(regFieldWidth);
    ImGui::InputScalar("##goto_address", ImGuiDataType_U32, &m_model.jumpAddress, nullptr, nullptr, "%08X",
                       ImGuiInputTextFlags_CharsHexadecimal);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        doJump();
    }
    ImGui::PopFont();

    ImGui::SameLine();
    if (ImGui::Button("跳转")) {
        doJump();
    }

    ImGui::SameLine();
    ImGui::Checkbox("跟随 PC", &m_model.followPC);

    ImGui::SameLine();
    ImGui::Checkbox("事件触发时", &m_model.followPCOnEvents);
    widgets::ExplanationTooltip("当命中断点和监视点时，使光标跳转到 PC。",
                                m_context.displayScale);

    ImGui::EndGroup();
}

} // namespace app::ui
