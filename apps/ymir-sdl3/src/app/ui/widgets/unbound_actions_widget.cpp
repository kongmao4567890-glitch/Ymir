#include "unbound_actions_widget.hpp"

namespace app::ui::widgets {

UnboundActionsWidget::UnboundActionsWidget(SharedContext &context)
    : m_context(context) {}

void UnboundActionsWidget::Display() {
    if (m_unboundActions.empty()) {
        ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight()));
        return;
    }

    const bool plural = m_unboundActions.size() > 1;
    ImGui::TextColored(m_context.colors.warn, "%zu %s", m_unboundActions.size(),
                       (plural ? "个操作已解除绑定" : "个操作已解除绑定"));
    ImGui::SameLine();
    if (ImGui::SmallButton("查看")) {
        ImGui::OpenPopup("已解除绑定的操作");
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("清除")) {
        m_unboundActions.clear();
    }

    if (ImGui::BeginPopup("已解除绑定的操作")) {
        for (auto &action : m_unboundActions) {
            const char *category;
            if (action.context == nullptr) {
                category = "快捷键";
            } else if (action.context == &m_context.controlPadInputs[0] ||
                       action.context == &m_context.analogPadInputs[0] ||
                       action.context == &m_context.arcadeRacerInputs[0] ||
                       action.context == &m_context.missionStickInputs[0] ||
                       action.context == &m_context.virtuaGunInputs[0] ||
                       action.context == &m_context.shuttleMouseInputs[0]) {
                category = "外设端口 1";
            } else if (action.context == &m_context.controlPadInputs[1] ||
                       action.context == &m_context.analogPadInputs[1] ||
                       action.context == &m_context.arcadeRacerInputs[1] ||
                       action.context == &m_context.missionStickInputs[1] ||
                       action.context == &m_context.virtuaGunInputs[1] ||
                       action.context == &m_context.shuttleMouseInputs[1]) {
                category = "外设端口 2";
            } else {
                category = "未知";
            }
            ImGui::Text("%s - %s - %s", category, action.action.group, action.action.name);
        }
        ImGui::EndPopup();
    }
}

void UnboundActionsWidget::Capture(const std::optional<input::MappedAction> &unboundAction) {
    m_unboundActions.clear();
    if (unboundAction) {
        m_unboundActions.insert(*unboundAction);
    }
}

void UnboundActionsWidget::Capture(const std::unordered_set<input::MappedAction> &unboundActions) {
    m_unboundActions.clear();
    m_unboundActions.insert(unboundActions.begin(), unboundActions.end());
}

} // namespace app::ui::widgets
