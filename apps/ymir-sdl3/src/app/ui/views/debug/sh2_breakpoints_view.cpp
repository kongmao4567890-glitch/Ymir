#include "sh2_breakpoints_view.hpp"

#include <ymir/hw/sh2/sh2.hpp>

#include <app/ui/fonts/IconsMaterialSymbols.h>

#include <app/events/emu_event_factory.hpp>

#include <imgui.h>

using namespace ymir;

namespace app::ui {

SH2BreakpointsView::SH2BreakpointsView(SharedContext &context, SH2DebuggerModel &model)
    : m_context(context)
    , m_model(model) {}

void SH2BreakpointsView::Display() {
    const float fontSize = m_context.fontSizes.medium;
    ImGui::PushFont(m_context.fonts.monospace.regular, fontSize);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();
    const float framePadding = ImGui::GetStyle().FramePadding.x;
    const float vecFieldWidth = framePadding * 2 + hexCharWidth * 8;

    auto drawHex32 = [&](auto id, uint32 &value) {
        ImGui::PushFont(m_context.fonts.monospace.regular, fontSize);
        ImGui::SetNextItemWidth(vecFieldWidth);
        ImGui::InputScalar(fmt::format("##input_{}", id).c_str(), ImGuiDataType_U32, &value, nullptr, nullptr, "%08X",
                           ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::PopFont();
        return ImGui::IsItemDeactivated();
    };

    ImGui::BeginGroup();

    if (!m_context.saturn.IsDebugTracingEnabled()) {
        ImGui::TextColored(m_context.colors.warn, "调试跟踪已禁用。");
        ImGui::TextColored(m_context.colors.warn, "断点将无法工作。");
        ImGui::SameLine();
        if (ImGui::SmallButton("启用##debug_tracing")) {
            m_context.EnqueueEvent(events::emu::SetDebugTrace(true));
        }
    }

    if (drawHex32("addr", m_address)) {
        m_address &= ~1u;
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
            ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown)) {

            std::unique_lock lock{m_context.locks.breakpoints};
            m_model.breakpoints.SetBreakpoint(m_address);
            m_context.debuggers.MakeDirty();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_ADD)) {
        std::unique_lock lock{m_context.locks.breakpoints};
        m_model.breakpoints.SetBreakpoint(m_address);
        m_context.debuggers.MakeDirty();
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("添加");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_REMOVE)) {
        std::unique_lock lock{m_context.locks.breakpoints};
        m_model.breakpoints.ClearBreakpoint(m_address);
        m_context.debuggers.MakeDirty();
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("移除");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_CLEAR_ALL)) {
        std::unique_lock lock{m_context.locks.breakpoints};
        m_model.breakpoints.ClearAllBreakpoints();
        m_context.debuggers.MakeDirty();
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("全部清除");
        ImGui::EndTooltip();
    }

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
    ImGui::SeparatorText("活跃断点");
    ImGui::PopFont();

    if (ImGui::BeginTable("bkpts", 4, ImGuiTableFlags_SizingFixedFit)) {
        const auto breakpoints = m_model.breakpoints.GetBreakpoints();
        for (auto &[baseAddress, bkpt] : breakpoints) {
            uint32 address = baseAddress;
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            bool enabled = bkpt.enabled;
            if (ImGui::Checkbox(fmt::format("##enabled_{}", baseAddress).c_str(), &enabled)) {
                std::unique_lock lock{m_context.locks.breakpoints};
                m_model.breakpoints.ToggleBreakpointEnabled(baseAddress);
                m_context.debuggers.MakeDirty();
            }
            if (ImGui::BeginItemTooltip()) {
                ImGui::TextUnformatted("启用/禁用断点");
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            if (drawHex32(address, address)) {
                std::unique_lock lock{m_context.locks.breakpoints};
                m_model.breakpoints.MoveBreakpoint(baseAddress, address);
                m_context.debuggers.MakeDirty();
            }

            ImGui::TableNextColumn();
            if (ImGui::Button(fmt::format(ICON_MS_JUMP_TO_ELEMENT "##{}", baseAddress).c_str())) {
                m_model.JumpTo(baseAddress);
            }
            if (ImGui::BeginItemTooltip()) {
                ImGui::TextUnformatted("跳转到地址");
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            if (ImGui::Button(fmt::format(ICON_MS_DELETE "##{}", baseAddress).c_str())) {
                std::unique_lock lock{m_context.locks.breakpoints};
                m_model.breakpoints.ClearBreakpoint(address);
                m_context.debuggers.MakeDirty();
            }
            if (ImGui::BeginItemTooltip()) {
                ImGui::TextUnformatted("移除");
                ImGui::EndTooltip();
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
