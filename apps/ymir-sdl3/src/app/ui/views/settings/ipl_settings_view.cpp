#include "ipl_settings_view.hpp"

#include <app/events/gui_event_factory.hpp>

#include <util/sdl_file_dialog.hpp>

#include <misc/cpp/imgui_stdlib.h>

#include <SDL3/SDL_misc.h>

#include <fmt/std.h>

using namespace ymir;

namespace app::ui {

static const char *GetVariantName(db::SystemVariant variant) {
    switch (variant) {
    case db::SystemVariant::None: return "无";
    case db::SystemVariant::Saturn: return "Saturn";
    case db::SystemVariant::HiSaturn: return "HiSaturn";
    case db::SystemVariant::VSaturn: return "V-Saturn";
    case db::SystemVariant::DevKit: return "开发套件";
    default: return "未知";
    }
}

static const char *GetRegionName(db::SystemRegion region) {
    switch (region) {
    case db::SystemRegion::None: return "无";
    case db::SystemRegion::US_EU: return "美国/欧洲";
    case db::SystemRegion::JP: return "日本";
    case db::SystemRegion::KR: return "韩国";
    default: return "未知";
    }
}

IPLSettingsView::IPLSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void IPLSettingsView::Display() {
    auto &settings = GetSettings().system.ipl;

    ImGui::TextUnformatted("注意：更改这些选项中的任何一个都将导致硬重置");

    ImGui::Separator();

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
    const float fileSelectorButtonWidth = ImGui::CalcTextSize("...").x + paddingWidth * 2;
    const float reloadButtonWidth = ImGui::CalcTextSize("重载").x + paddingWidth * 2;
    const float useButtonWidth = ImGui::CalcTextSize("使用").x + paddingWidth * 2;

    std::filesystem::path iplRomsPath = m_context.profile.GetPath(ProfilePath::IPLROMImages);

    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
    ImGui::Text("%s 中的 IPL ROM", fmt::format("{}", iplRomsPath).c_str());
    ImGui::PopTextWrapPos();

    if (ImGui::Button("打开目录")) {
        SDL_OpenURL(fmt::format("file:///{}", iplRomsPath).c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("重新扫描")) {
        {
            std::unique_lock lock{m_context.locks.romManager};
            m_context.romManager.ScanIPLROMs(iplRomsPath);
        }
        if (m_context.iplRomPath.empty() && !m_context.romManager.GetIPLROMs().empty()) {
            m_context.EnqueueEvent(events::gui::ReloadIPLROM());
        }
    }

    int index = 0;
    if (ImGui::BeginTable("sys_ipl_roms", 6,
                          ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti |
                              ImGuiTableFlags_SortTristate,
                          ImVec2(0, 250 * m_context.displayScale))) {
        ImGui::TableSetupColumn("路径", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 0.0f);
        ImGui::TableSetupColumn("版本", ImGuiTableColumnFlags_WidthFixed, 50 * m_context.displayScale);
        ImGui::TableSetupColumn("日期", ImGuiTableColumnFlags_WidthFixed, 75 * m_context.displayScale);
        ImGui::TableSetupColumn("变体", ImGuiTableColumnFlags_WidthFixed, 60 * m_context.displayScale);
        ImGui::TableSetupColumn("区域", ImGuiTableColumnFlags_WidthFixed, 105 * m_context.displayScale);
        ImGui::TableSetupColumn("##use", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                useButtonWidth);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        std::vector<IPLROMEntry> sortedIpl;

        for (const auto &[path, info] : m_context.romManager.GetIPLROMs()) {
            sortedIpl.emplace_back(info);
        }

        if (const ImGuiTableSortSpecs *sortSpecs = ImGui::TableGetSortSpecs();
            sortSpecs->SpecsDirty && sortedIpl.size() > 1) {

            for (int specIndex = sortSpecs->SpecsCount - 1; specIndex >= 0; --specIndex) {
                const ImGuiTableColumnSortSpecs &sortSpec = sortSpecs->Specs[specIndex];

                const auto sortColumns = [&sortSpec](auto sortStart, auto sortEnd) -> void {
                    switch (sortSpec.ColumnIndex) {
                    case 0: // Path
                        std::stable_sort(sortStart, sortEnd, [](const IPLROMEntry &lhs, const IPLROMEntry &rhs) {
                            return lhs.path < rhs.path;
                        });
                        break;
                    case 1: // Version
                        std::stable_sort(sortStart, sortEnd, [](const IPLROMEntry &lhs, const IPLROMEntry &rhs) {
                            return lhs.versionString < rhs.versionString;
                        });
                        break;
                    case 2: // Date
                        std::stable_sort(sortStart, sortEnd, [](const IPLROMEntry &lhs, const IPLROMEntry &rhs) {
                            if (lhs.info && rhs.info) {
                                return std::tie(lhs.info->year, lhs.info->month, lhs.info->day) <
                                       std::tie(rhs.info->year, rhs.info->month, rhs.info->day);
                            } else {
                                return (lhs.info != nullptr) < (rhs.info != nullptr);
                            }
                        });
                        break;
                    case 3: // Variant
                        std::stable_sort(sortStart, sortEnd, [](const IPLROMEntry &lhs, const IPLROMEntry &rhs) {
                            if (lhs.info && rhs.info) {
                                return (lhs.info->variant < rhs.info->variant);
                            } else {
                                return (lhs.info != nullptr) < (rhs.info != nullptr);
                            }
                        });
                        break;
                    case 4: // Region
                        std::stable_sort(sortStart, sortEnd, [](const IPLROMEntry &lhs, const IPLROMEntry &rhs) {
                            if (lhs.info && rhs.info) {
                                return std::tie(lhs.info->regionFree, lhs.info->region) <
                                       std::tie(rhs.info->regionFree, rhs.info->region);
                            } else {
                                return (lhs.info != nullptr) < (rhs.info != nullptr);
                            }
                        });
                        break;
                    case 5: // ##Use
                        break;
                    default: util::unreachable();
                    }
                };

                switch (sortSpec.SortDirection) {
                case ImGuiSortDirection_None: break;
                case ImGuiSortDirection_Ascending: sortColumns(sortedIpl.begin(), sortedIpl.end()); break;
                case ImGuiSortDirection_Descending: sortColumns(sortedIpl.rbegin(), sortedIpl.rend()); break;
                }
            }
        }

        for (const auto &iplRom : sortedIpl) {
            ImGui::TableNextRow();

            if (ImGui::TableNextColumn()) {
                std::filesystem::path relativePath = std::filesystem::relative(iplRom.path, iplRomsPath);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", fmt::format("{}", relativePath).c_str());
            }
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                if (iplRom.info != nullptr) {
                    ImGui::Text("%s", iplRom.info->version);
                } else {
                    ImGui::TextUnformatted("-");
                }
            }
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                if (iplRom.info != nullptr) {
                    ImGui::Text("%04u/%02u/%02u", iplRom.info->year, iplRom.info->month, iplRom.info->day);
                } else {
                    ImGui::TextUnformatted("-");
                }
            }
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                if (iplRom.info != nullptr) {
                    ImGui::Text("%s", GetVariantName(iplRom.info->variant));
                } else {
                    ImGui::TextUnformatted("未知");
                }
            }
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                if (iplRom.info != nullptr) {
                    if (iplRom.info->regionFree) {
                        ImGui::Text("%s (RF)", GetRegionName(iplRom.info->region));
                    } else {
                        ImGui::Text("%s", GetRegionName(iplRom.info->region));
                    }
                } else {
                    ImGui::TextUnformatted("未知");
                }
            }
            if (ImGui::TableNextColumn()) {
                if (ImGui::Button(fmt::format("使用##{}", index).c_str())) {
                    settings.overrideImage = true;
                    settings.path = iplRom.path;
                    if (!settings.path.empty()) {
                        m_context.EnqueueEvent(events::gui::ReloadIPLROM());
                        MakeDirty();
                    }
                }
            }
            ++index;
        }

        ImGui::EndTable();
    }

    ImGui::Separator();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("首选系统变体");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##variant", GetVariantName(settings.variant), ImGuiComboFlags_WidthFitPreview)) {
        for (int i = 0; i <= 4; ++i) {
            const auto variant = static_cast<db::SystemVariant>(i);
            if (MakeDirty(ImGui::Selectable(GetVariantName(variant), variant == settings.variant))) {
                settings.variant = variant;
                m_context.EnqueueEvent(events::gui::ReloadIPLROM());
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    if (MakeDirty(ImGui::Checkbox("覆盖 IPL ROM", &settings.overrideImage))) {
        if (settings.overrideImage && !settings.path.empty()) {
            m_context.EnqueueEvent(events::gui::ReloadIPLROM());
            MakeDirty();
        }
    }

    if (!settings.overrideImage) {
        ImGui::BeginDisabled();
    }
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("IPL ROM 路径");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-(fileSelectorButtonWidth + reloadButtonWidth + itemSpacingWidth * 2));
    std::string iplPath = fmt::format("{}", settings.path);
    if (MakeDirty(ImGui::InputText("##ipl_path", &iplPath, ImGuiInputTextFlags_ElideLeft))) {
        settings.path = std::u8string{iplPath.begin(), iplPath.end()};
    }
    ImGui::SameLine();
    if (ImGui::Button("...##ipl_path")) {
        m_context.EnqueueEvent(events::gui::OpenFile({
            .dialogTitle = "加载 IPL ROM",
            .filters = {{"ROM 文件 (*.bin, *.rom)", "bin;rom"}, {"所有文件 (*.*)", "*"}},
            .userdata = this,
            .callback = util::WrapSingleSelectionCallback<&IPLSettingsView::ProcessLoadIPLROM,
                                                          &util::NoopCancelFileDialogCallback,
                                                          &IPLSettingsView::ProcessLoadIPLROMError>,
        }));
    }
    ImGui::SameLine();
    if (ImGui::Button("重载")) {
        if (!settings.path.empty()) {
            m_context.EnqueueEvent(events::gui::ReloadIPLROM());
            MakeDirty();
        }
    }
    if (!settings.overrideImage) {
        ImGui::EndDisabled();
    }

    ImGui::Separator();

    if (m_context.iplRomPath.empty()) {
        ImGui::TextUnformatted("未加载 IPL ROM");
    } else {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::Text("当前使用的 IPL ROM 位于 %s", fmt::format("{}", m_context.iplRomPath).c_str());
        ImGui::PopTextWrapPos();
    }
    const db::IPLROMInfo *info = db::GetIPLROMInfo(m_context.saturn.GetIPLHash());
    if (info != nullptr) {
        ImGui::Text("版本：%s", info->version);
        ImGui::Text("发布日期：%04u/%02u/%02u", info->year, info->month, info->day);
        ImGui::Text("变体：%s", GetVariantName(info->variant));
        ImGui::Text("区域：%s", GetRegionName(info->region));
    } else {
        ImGui::TextUnformatted("未知 IPL ROM");
    }
}

void IPLSettingsView::ProcessLoadIPLROM(void *userdata, std::filesystem::path file, int filter) {
    static_cast<IPLSettingsView *>(userdata)->LoadIPLROM(file);
}

void IPLSettingsView::ProcessLoadIPLROMError(void *userdata, const char *message, int filter) {
    static_cast<IPLSettingsView *>(userdata)->ShowIPLROMLoadError(message);
}

void IPLSettingsView::LoadIPLROM(std::filesystem::path file) {
    m_context.EnqueueEvent(events::gui::TryLoadIPLROM(file));
}

void IPLSettingsView::ShowIPLROMLoadError(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(fmt::format("无法加载 IPL ROM：{}", message)));
}

} // namespace app::ui
