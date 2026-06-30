#include "cheat_manager_window.hpp"

#include <app/ui/widgets/common_widgets.hpp>

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace app::ui {

CheatManagerWindow::CheatManagerWindow(SharedContext &context, CheatManager &cheatManager)
    : WindowBase(context), m_cheatManager(cheatManager) {
    m_windowConfig.name = "金手指管理器";
    m_windowConfig.flags = ImGuiWindowFlags_None;
}

void CheatManagerWindow::DrawContents() {
    auto &cheats = m_cheatManager.GetCheatsMut();

    // 说明区域
    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("批量添加金手指");
    ImGui::PopFont();

    ImGui::TextWrapped("在下方输入金手指名称和 Action Replay 格式的代码（每行一条，格式: 地址 数值）。");
    ImGui::TextWrapped("示例: 06000000 0064  (将高 WRAM 地址 0x06000000 的值设为 100)");
    ImGui::TextWrapped("支持多行批量输入，所有代码将合并为一条金手指。");

    ImGui::Spacing();

    // 名称输入
    ImGui::TextUnformatted("名称:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300 * m_context.displayScale);
    ImGui::InputText("##cheatName", m_nameBuf, sizeof(m_nameBuf));

    // 代码批量输入
    ImGui::TextUnformatted("代码:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextMultiline("##cheatCodes", m_codeBuf, sizeof(m_codeBuf),
                              ImVec2(-1, 150 * m_context.displayScale));

    ImGui::Spacing();

    // 添加按钮
    if (ImGui::Button("添加金手指", ImVec2(120 * m_context.displayScale, 0))) {
        std::string name(m_nameBuf);
        std::string codeText(m_codeBuf);

        if (!name.empty() && !codeText.empty()) {
            auto codes = CheatManager::ParseActionReplay(codeText);
            if (!codes.empty()) {
                CheatEntry entry;
                entry.name = name;
                entry.codes = codes;
                entry.enabled = true;
                m_cheatManager.AddCheat(entry);

                // 清空输入
                std::memset(m_nameBuf, 0, sizeof(m_nameBuf));
                std::memset(m_codeBuf, 0, sizeof(m_codeBuf));

                m_context.DisplayMessage("[金手指] 已添加: " + name + " (" + std::to_string(codes.size()) + " 条代码)");
            } else {
                m_context.DisplayMessage("[金手指] 无法解析代码，请检查格式");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("清空输入", ImVec2(100 * m_context.displayScale, 0))) {
        std::memset(m_nameBuf, 0, sizeof(m_nameBuf));
        std::memset(m_codeBuf, 0, sizeof(m_codeBuf));
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 金手指列表
    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("金手指列表");
    ImGui::PopFont();

    {
        std::lock_guard lock(m_cheatManager.GetMutex());

        if (cheats.empty()) {
            ImGui::TextDisabled("（暂无金手指代码）");
        } else {
            // 列表头
            if (ImGui::BeginTable("cheatList", 4,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("启用", ImGuiTableColumnFlags_WidthFixed, 50 * m_context.displayScale);
                ImGui::TableSetupColumn("名称", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("代码数", ImGuiTableColumnFlags_WidthFixed, 60 * m_context.displayScale);
                ImGui::TableSetupColumn("操作", ImGuiTableColumnFlags_WidthFixed, 120 * m_context.displayScale);
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < cheats.size(); i++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    // 启用/禁用复选框
                    char checkboxId[32];
                    snprintf(checkboxId, sizeof(checkboxId), "##enable_%zu", i);
                    bool enabled = cheats[i].enabled;
                    if (ImGui::Checkbox(checkboxId, &enabled)) {
                        cheats[i].enabled = enabled;
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(cheats[i].name.c_str());

                    // 可选：显示代码详情（可折叠）
                    if (ImGui::TreeNode((void *)(intptr_t)i, "详情")) {
                        for (size_t j = 0; j < cheats[i].codes.size(); j++) {
                            auto &code = cheats[i].codes[j];
                            ImGui::Text("  %08X %04X", code.address, code.value);
                        }
                        ImGui::TreePop();
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%zu", cheats[i].codes.size());

                    ImGui::TableNextColumn();
                    // 删除按钮
                    char delBtnId[32];
                    snprintf(delBtnId, sizeof(delBtnId), "删除##del_%zu", i);
                    if (ImGui::SmallButton(delBtnId)) {
                        m_cheatManager.RemoveCheat(i);
                        break; // 迭代器失效，退出循环
                    }
                }
                ImGui::EndTable();
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 底部操作
    if (ImGui::Button("全部启用", ImVec2(100 * m_context.displayScale, 0))) {
        std::lock_guard lock(m_cheatManager.GetMutex());
        for (auto &c : m_cheatManager.GetCheatsMut()) {
            c.enabled = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("全部禁用", ImVec2(100 * m_context.displayScale, 0))) {
        std::lock_guard lock(m_cheatManager.GetMutex());
        for (auto &c : m_cheatManager.GetCheatsMut()) {
            c.enabled = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("清空全部", ImVec2(100 * m_context.displayScale, 0))) {
        m_cheatManager.ClearAll();
        m_context.DisplayMessage("[金手指] 已清空所有金手指");
    }

    ImGui::Spacing();
    ImGui::TextDisabled("提示: 金手指在模拟运行时每帧自动应用。修改后立即生效。");
}

} // namespace app::ui
