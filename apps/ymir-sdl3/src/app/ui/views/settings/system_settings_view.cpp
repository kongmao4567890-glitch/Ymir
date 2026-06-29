#include "system_settings_view.hpp"

#include <ymir/hw/smpc/smpc.hpp>

#include <app/events/emu_event_factory.hpp>
#include <app/events/gui_event_factory.hpp>

#include <app/ui/widgets/common_widgets.hpp>
#include <app/ui/widgets/datetime_widgets.hpp>
#include <app/ui/widgets/settings_widgets.hpp>
#include <app/ui/widgets/system_widgets.hpp>

#include <util/regions.hpp>
#include <util/sdl_file_dialog.hpp>

#include <ymir/util/size_ops.hpp>

#include <misc/cpp/imgui_stdlib.h>

#include <fmt/format.h>
#include <fmt/std.h>

#include <SDL3/SDL_misc.h>

#include <set>

using namespace ymir;

namespace app::ui {

SystemSettingsView::SystemSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void SystemSettingsView::Display() {
    auto &settings = GetSettings().system;

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
    const float fileSelectorButtonWidth = ImGui::CalcTextSize("...").x + paddingWidth * 2;

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("区域");
    ImGui::PopFont();

    if (ImGui::BeginTable("sys_region", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 0);
        ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("视频制式");
        }
        if (ImGui::TableNextColumn()) {
            MakeDirty(ui::widgets::VideoStandardSelector(m_context));
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("区域");
            widgets::ExplanationTooltip("更改此选项将导致硬重置", m_context.displayScale);
        }
        if (ImGui::TableNextColumn()) {
            ui::widgets::RegionSelector(m_context);
        }

        ImGui::EndTable();
    }

    bool autodetectRegion = settings.autodetectRegion.Get();
    if (MakeDirty(ImGui::Checkbox("从加载的光盘自动检测区域", &autodetectRegion))) {
        settings.autodetectRegion = autodetectRegion;
    }
    widgets::ExplanationTooltip(
        "每当加载游戏光盘时，模拟器将自动切换系统区域以匹配游戏支持的某个区域。"
        "下面的列表允许你选择首选区域顺序。如果游戏不支持任何首选区域，模拟器将选择光盘上列出的第一个区域。",
        m_context.displayScale);

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("首选区域顺序：");
    widgets::ExplanationTooltip("拖动项目以重新排序", m_context.displayScale);

    std::vector<core::config::sys::Region> prefRgnOrder{};
    {
        // Set of all valid regions
        std::set<core::config::sys::Region> validRegions{
            core::config::sys::Region::Japan, core::config::sys::Region::NorthAmerica,
            core::config::sys::Region::AsiaNTSC, core::config::sys::Region::EuropePAL};

        // Build list of regions from setting using only valid options
        for (auto &region : settings.preferredRegionOrder.Get()) {
            if (validRegions.erase(region)) {
                prefRgnOrder.push_back(region);
            }
        }

        // Add any missing regions to the end of the list
        prefRgnOrder.insert(prefRgnOrder.end(), validRegions.begin(), validRegions.end());
    }

    if (ImGui::BeginListBox("##pref_rgn_order", ImVec2(150 * m_context.displayScale, ImGui::GetFrameHeight() * 4))) {
        ImGui::PushItemFlag(ImGuiItemFlags_AllowDuplicateId, true);
        bool changed = false;
        for (int n = 0; n < prefRgnOrder.size(); n++) {
            core::config::sys::Region item = prefRgnOrder[n];
            ImGui::Selectable(util::RegionToString(item).c_str());

            if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
                int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                if (n_next >= 0 && n_next < prefRgnOrder.size()) {
                    prefRgnOrder[n] = prefRgnOrder[n_next];
                    prefRgnOrder[n_next] = item;
                    ImGui::ResetMouseDragDelta();
                    changed = true;
                }
            }
        }
        ImGui::PopItemFlag();

        if (changed) {
            settings.preferredRegionOrder = prefRgnOrder;
            MakeDirty();
        }

        ImGui::EndListBox();
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("SH-2");
    ImGui::PopFont();

    widgets::settings::system::EmulateSH2Cache(m_context);
    widgets::settings::system::SH2ClockFactor(m_context);

    // -----------------------------------------------------------------------------------------------------------------

    auto &smpc = m_context.saturn.GetSMPC();
    auto &rtc = smpc.GetRTC();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("实时时钟");
    ImGui::PopFont();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("模式：");
    widgets::ExplanationTooltip("- 主机：将模拟的 RTC 同步到你系统的时钟。\n"
                                "- 虚拟：运行与模拟速度同步的虚拟 RTC。\n\n"
                                "为了获得确定性行为，请使用在重置时同步到固定时间点的虚拟 RTC。",
                                m_context.displayScale);
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("主机##rtc", settings.rtc.mode == core::config::rtc::Mode::Host))) {
        settings.rtc.mode = core::config::rtc::Mode::Host;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("虚拟##rtc", settings.rtc.mode == core::config::rtc::Mode::Virtual))) {
        settings.rtc.mode = core::config::rtc::Mode::Virtual;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("当前日期/时间：");
    ImGui::SameLine();
    auto dateTime = rtc.GetDateTime();
    if (widgets::DateTimeSelector("rtc_curr", dateTime)) {
        rtc.SetDateTime(dateTime);
        smpc.PersistData();
    }

    if (settings.rtc.mode == core::config::rtc::Mode::Host) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("主机时间偏移：");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f * m_context.displayScale);
        if (ImGui::DragScalar("##rtc_host_offset", ImGuiDataType_S64, &rtc.HostTimeOffset())) {
            smpc.PersistData();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("秒");
        ImGui::SameLine();
        if (ImGui::Button("重置")) {
            rtc.HostTimeOffset() = 0;
            smpc.PersistData();
        }
    } else if (settings.rtc.mode == core::config::rtc::Mode::Virtual) {
        // TODO: request emulator to update date/time so that it is updated in real time
        widgets::ExplanationTooltip(
            "这可能偶尔会停止更新，因为虚拟 RTC 仅在游戏读取时才更新。",
            m_context.displayScale);

        if (ImGui::Button("设为主机时间##curr_time")) {
            rtc.SetDateTime(util::datetime::host());
            smpc.PersistData();
        }
        ImGui::SameLine();
        if (ImGui::Button("设为起始点##curr_time")) {
            rtc.SetDateTime(util::datetime::from_timestamp(settings.rtc.virtHardResetTimestamp));
            smpc.PersistData();
        }

        using HardResetStrategy = core::config::rtc::HardResetStrategy;

        auto hardResetOption = [&](const char *name, HardResetStrategy strategy, const char *explanation) {
            if (MakeDirty(ImGui::RadioButton(fmt::format("{}##virt_rtc_reset", name).c_str(),
                                             settings.rtc.virtHardResetStrategy == strategy))) {
                settings.rtc.virtHardResetStrategy = strategy;
            }
            widgets::ExplanationTooltip(explanation, m_context.displayScale);
        };

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("硬重置行为：");
        widgets::ExplanationTooltip("指定虚拟 RTC 在硬重置时的行为。", m_context.displayScale);

        hardResetOption("保留当前时间", HardResetStrategy::Preserve,
                        "虚拟 RTC 将从重置前的时间点继续计数。\n"
                        "日期/时间在模拟器的多次运行之间保持不变。");

        hardResetOption("同步到主机时间", HardResetStrategy::SyncToHost,
                        "虚拟 RTC 将重置为当前主机 RTC 时间。");

        hardResetOption("重置到起始点", HardResetStrategy::ResetToFixedTime,
                        "虚拟 RTC 将重置到指定的起始点。");

        ImGui::Indent();
        {
            auto dateTime = util::datetime::from_timestamp(settings.rtc.virtHardResetTimestamp);
            if (MakeDirty(widgets::DateTimeSelector("virt_base_time", dateTime))) {
                settings.rtc.virtHardResetTimestamp = util::datetime::to_timestamp(dateTime);
            }
            if (MakeDirty(ImGui::Button("设为主机时间##virt_base_time"))) {
                settings.rtc.virtHardResetTimestamp = util::datetime::to_timestamp(util::datetime::host());
            }
        }
        ImGui::Unindent();
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("内部备份内存");
    ImGui::PopFont();

    if (MakeDirty(
            ImGui::Checkbox("为每个游戏创建内部备份内存镜像", &settings.internalBackupRAMPerGame))) {
        m_context.EnqueueEvent(events::emu::LoadInternalBackupMemory());
    }
    widgets::ExplanationTooltip(
        fmt::format("启用后，将在 {} 下为每个游戏创建单独的内部备份内存镜像",
                    m_context.profile.GetPath(ProfilePath::BackupMemory) / "games")
            .c_str(),
        m_context.displayScale);

    if (settings.internalBackupRAMPerGame) {
        ImGui::BeginDisabled();
    }
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("镜像路径");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-(fileSelectorButtonWidth + itemSpacingWidth));
    std::string imagePath = fmt::format("{}", settings.internalBackupRAMImagePath);
    if (MakeDirty(ImGui::InputText("##bup_image_path", &imagePath, ImGuiInputTextFlags_ElideLeft))) {
        settings.internalBackupRAMImagePath = std::u8string{imagePath.begin(), imagePath.end()};
        m_bupSettingsDirty = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("...##bup_image_path")) {
        m_context.EnqueueEvent(events::gui::OpenFile({
            .dialogTitle = "加载备份内存镜像",
            .defaultPath = settings.internalBackupRAMImagePath.empty()
                               ? m_context.profile.GetPath(ProfilePath::PersistentState) / "bup-int.bin"
                               : settings.internalBackupRAMImagePath,
            .filters = {{"备份内存镜像文件 (*.bin, *.sav)", "bin;sav"}, {"所有文件 (*.*)", "*"}},
            .userdata = this,
            .callback = util::WrapSingleSelectionCallback<&SystemSettingsView::ProcessLoadBackupImage,
                                                          &util::NoopCancelFileDialogCallback,
                                                          &SystemSettingsView::ProcessLoadBackupImageError>,
        }));
    }
    if (settings.internalBackupRAMPerGame) {
        ImGui::EndDisabled();
    }

    const bool dirty = m_bupSettingsDirty;
    if (!dirty) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("加载")) {
        m_context.EnqueueEvent(events::emu::LoadInternalBackupMemory());
        m_bupSettingsDirty = false;
    }
    if (!dirty) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (ImGui::Button("打开备份内存管理器")) {
        m_context.EnqueueEvent(events::gui::OpenBackupMemoryManager());
    }

    if (settings.internalBackupRAMPerGame) {
        const std::filesystem::path intBupPath = m_context.GetInternalBackupRAMPath();
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::Text("当前使用的内部备份内存镜像来自 %s", fmt::format("{}", intBupPath).c_str());
        ImGui::PopTextWrapPos();
        if (ImGui::Button("打开所在目录##int_bup")) {
            SDL_OpenURL(fmt::format("file:///{}", intBupPath.parent_path()).c_str());
        }
    }
}

void SystemSettingsView::ProcessLoadBackupImage(void *userdata, std::filesystem::path file, int filter) {
    static_cast<SystemSettingsView *>(userdata)->LoadBackupImage(file);
}

void SystemSettingsView::ProcessLoadBackupImageError(void *userdata, const char *message, int filter) {
    static_cast<SystemSettingsView *>(userdata)->ShowLoadBackupImageError(message);
}

void SystemSettingsView::LoadBackupImage(std::filesystem::path file) {
    auto &settings = GetSettings().system;

    if (std::filesystem::is_regular_file(file)) {
        // The user selected an existing image. Make sure it's a proper internal backup image.
        std::error_code error{};
        bup::BackupMemory bupMem{};
        const auto result = bupMem.LoadFrom(file, false, error);
        switch (result) {
        case bup::BackupMemoryImageLoadResult::Success:
            if (bupMem.Size() == 32_KiB) {
                settings.internalBackupRAMImagePath = file;
                m_bupSettingsDirty = false;
                m_context.EnqueueEvent(events::emu::LoadInternalBackupMemory());
                MakeDirty();
            } else {
                m_context.EnqueueEvent(
                    events::gui::ShowError(fmt::format("无法加载备份内存镜像：镜像大小无效")));
            }
            break;
        case bup::BackupMemoryImageLoadResult::FilesystemError:
            if (error) {
                m_context.EnqueueEvent(
                    events::gui::ShowError(fmt::format("无法加载备份内存镜像：{}", error.message())));
            } else {
                m_context.EnqueueEvent(events::gui::ShowError(
                    fmt::format("无法加载备份内存镜像：未指定的文件系统错误")));
            }
            break;
        case bup::BackupMemoryImageLoadResult::InvalidSize:
            m_context.EnqueueEvent(
                events::gui::ShowError(fmt::format("无法加载备份内存镜像：镜像大小无效")));
            break;
        default:
            m_context.EnqueueEvent(
                events::gui::ShowError(fmt::format("无法加载备份内存镜像：意外错误")));
            break;
        }
    } else {
        // The user wants to create a new image file
        settings.internalBackupRAMImagePath = file;
        m_bupSettingsDirty = false;
        m_context.EnqueueEvent(events::emu::LoadInternalBackupMemory());
        MakeDirty();
    }
}

void SystemSettingsView::ShowLoadBackupImageError(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(fmt::format("无法加载备份内存镜像：{}", message)));
}

} // namespace app::ui
