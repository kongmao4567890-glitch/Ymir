#include "cdblock_settings_view.hpp"

#include <app/events/gui_event_factory.hpp>

#include <app/ui/widgets/settings_widgets.hpp>

#include <util/sdl_file_dialog.hpp>

#include <misc/cpp/imgui_stdlib.h>

#include <SDL3/SDL_misc.h>

using namespace ymir;

namespace app::ui {

CDBlockSettingsView::CDBlockSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void CDBlockSettingsView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
    const float fileSelectorButtonWidth = ImGui::CalcTextSize("...").x + paddingWidth * 2;
    const float reloadButtonWidth = ImGui::CalcTextSize("重载").x + paddingWidth * 2;
    const float useButtonWidth = ImGui::CalcTextSize("使用").x + paddingWidth * 2;

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("精度");
    ImGui::PopFont();

    ImGui::TextUnformatted("注意：更改这些选项中的任何一个都将导致硬重置");

    widgets::settings::cdblock::CDBlockLLE(m_context);

    ImGui::Separator();

    std::filesystem::path cdbRomsPaths = m_context.profile.GetPath(ProfilePath::CDBlockROMImages);

    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
    ImGui::Text("%s 中的 CD block ROM", fmt::format("{}", cdbRomsPaths).c_str());
    ImGui::PopTextWrapPos();

    if (ImGui::Button("打开目录")) {
        SDL_OpenURL(fmt::format("file:///{}", cdbRomsPaths).c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("重新扫描")) {
        {
            std::unique_lock lock{m_context.locks.romManager};
            m_context.romManager.ScanCDBlockROMs(cdbRomsPaths);
        }
        if (m_context.cdbRomPath.empty() && !m_context.romManager.GetCDBlockROMs().empty()) {
            m_context.EnqueueEvent(events::gui::ReloadCDBlockROM());
        }
    }

    auto &settings = GetSettings().cdblock;

    int index = 0;
    if (ImGui::BeginTable("cdb_roms", 3,
                          ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti |
                              ImGuiTableFlags_SortTristate,
                          ImVec2(0, 150 * m_context.displayScale))) {
        ImGui::TableSetupColumn("路径", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 0.0f);
        ImGui::TableSetupColumn("版本", ImGuiTableColumnFlags_WidthFixed, 60 * m_context.displayScale);
        ImGui::TableSetupColumn("##use", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                useButtonWidth);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        std::vector<CDBlockROMEntry> sortedROMs;

        for (const auto &[path, info] : m_context.romManager.GetCDBlockROMs()) {
            sortedROMs.emplace_back(info);
        }

        if (const ImGuiTableSortSpecs *sortSpecs = ImGui::TableGetSortSpecs();
            sortSpecs->SpecsDirty && sortedROMs.size() > 1) {

            for (int specIndex = sortSpecs->SpecsCount - 1; specIndex >= 0; --specIndex) {
                const ImGuiTableColumnSortSpecs &sortSpec = sortSpecs->Specs[specIndex];

                const auto sortColumns = [&sortSpec](auto sortStart, auto sortEnd) -> void {
                    switch (sortSpec.ColumnIndex) {
                    case 0: // Path
                        std::stable_sort(
                            sortStart, sortEnd,
                            [](const CDBlockROMEntry &lhs, const CDBlockROMEntry &rhs) { return lhs.path < rhs.path; });
                        break;
                    case 1: // Version
                        std::stable_sort(sortStart, sortEnd,
                                         [](const CDBlockROMEntry &lhs, const CDBlockROMEntry &rhs) {
                                             if (lhs.info == nullptr || rhs.info == nullptr) {
                                                 return false;
                                             }
                                             return lhs.info->version < rhs.info->version;
                                         });
                        break;
                    case 3: // ##Use
                        break;
                    default: util::unreachable();
                    }
                };

                switch (sortSpec.SortDirection) {
                case ImGuiSortDirection_None: break;
                case ImGuiSortDirection_Ascending: sortColumns(sortedROMs.begin(), sortedROMs.end()); break;
                case ImGuiSortDirection_Descending: sortColumns(sortedROMs.rbegin(), sortedROMs.rend()); break;
                }
            }
        }

        for (const auto &cdbROM : sortedROMs) {
            ImGui::TableNextRow();

            if (ImGui::TableNextColumn()) {
                std::filesystem::path relativePath = std::filesystem::relative(cdbROM.path, cdbRomsPaths);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", fmt::format("{}", relativePath).c_str());
            }
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                if (cdbROM.info != nullptr) {
                    ImGui::Text("%s", cdbROM.info->version.data());
                } else {
                    ImGui::TextUnformatted("-");
                }
            }
            if (ImGui::TableNextColumn()) {
                if (ImGui::Button(fmt::format("使用##{}", index).c_str())) {
                    settings.overrideROM = true;
                    settings.romPath = cdbROM.path;
                    if (!settings.romPath.empty()) {
                        m_context.EnqueueEvent(events::gui::ReloadCDBlockROM());
                        MakeDirty();
                    }
                }
            }
            ++index;
        }

        ImGui::EndTable();
    }

    ImGui::Separator();

    if (MakeDirty(ImGui::Checkbox("覆盖 CD block ROM", &settings.overrideROM))) {
        if (settings.overrideROM && !settings.romPath.empty()) {
            m_context.EnqueueEvent(events::gui::ReloadCDBlockROM());
            MakeDirty();
        }
    }

    if (!settings.overrideROM) {
        ImGui::BeginDisabled();
    }
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("CD block ROM 路径");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-(fileSelectorButtonWidth + reloadButtonWidth + itemSpacingWidth * 2));
    std::string cdbPath = fmt::format("{}", settings.romPath);
    if (MakeDirty(ImGui::InputText("##cdb_path", &cdbPath, ImGuiInputTextFlags_ElideLeft))) {
        settings.romPath = std::u8string{cdbPath.begin(), cdbPath.end()};
    }
    ImGui::SameLine();
    if (ImGui::Button("...##cdb_path")) {
        m_context.EnqueueEvent(events::gui::OpenFile({
            .dialogTitle = "加载 CD block ROM",
            .filters = {{"ROM 文件 (*.bin, *.rom)", "bin;rom"}, {"所有文件 (*.*)", "*"}},
            .userdata = this,
            .callback = util::WrapSingleSelectionCallback<&CDBlockSettingsView::ProcessLoadCDBlockROM,
                                                          &util::NoopCancelFileDialogCallback,
                                                          &CDBlockSettingsView::ProcessLoadCDBlockROMError>,
        }));
    }
    ImGui::SameLine();
    if (ImGui::Button("重载")) {
        if (!settings.romPath.empty()) {
            m_context.EnqueueEvent(events::gui::ReloadCDBlockROM());
            MakeDirty();
        }
    }
    if (!settings.overrideROM) {
        ImGui::EndDisabled();
    }

    ImGui::Separator();

    if (m_context.cdbRomPath.empty()) {
        ImGui::TextUnformatted("未加载 CD block ROM");
    } else {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::Text("当前使用的 CD block ROM 位于 %s", fmt::format("{}", m_context.cdbRomPath).c_str());
        ImGui::PopTextWrapPos();
    }
    const db::CDBlockROMInfo *info = db::GetCDBlockROMInfo(m_context.saturn.GetCDBlockROMHash());
    if (info != nullptr) {
        ImGui::Text("版本：%s", info->version.data());
    } else {
        ImGui::TextUnformatted("未知 CD block ROM");
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("调整");
    ImGui::PopFont();

    widgets::settings::cdblock::CDReadSpeed(m_context);
}

void CDBlockSettingsView::ProcessLoadCDBlockROM(void *userdata, std::filesystem::path file, int filter) {
    static_cast<CDBlockSettingsView *>(userdata)->LoadCDBlockROM(file);
}

void CDBlockSettingsView::ProcessLoadCDBlockROMError(void *userdata, const char *message, int filter) {
    static_cast<CDBlockSettingsView *>(userdata)->ShowCDBlockROMLoadError(message);
}

void CDBlockSettingsView::LoadCDBlockROM(std::filesystem::path file) {
    m_context.EnqueueEvent(events::gui::TryLoadCDBlockROM(file));
}

void CDBlockSettingsView::ShowCDBlockROMLoadError(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(fmt::format("无法加载 CD block ROM：{}", message)));
}

} // namespace app::ui
