#include "general_settings_view.hpp"

#include <app/events/emu_event_factory.hpp>
#include <app/events/gui_event_factory.hpp>

#include <app/ui/widgets/common_widgets.hpp>

#include <misc/cpp/imgui_stdlib.h>

#include <util/math.hpp>
#include <util/sdl_file_dialog.hpp>

#include <fmt/std.h>

#include <SDL3/SDL_misc.h>

namespace app::ui {

GeneralSettingsView::GeneralSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void GeneralSettingsView::Display() {
    auto &settings = GetSettings().general;
    auto &profile = m_context.profile;

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
    const float fileSelectorButtonWidth = ImGui::CalcTextSize("...").x + paddingWidth * 2;
    const float clearButtonWidth = ImGui::CalcTextSize("清除").x + paddingWidth * 2;
    const float openButtonWidth = ImGui::CalcTextSize("打开").x + paddingWidth * 2;

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("性能");
    ImGui::PopFont();

    if (MakeDirty(ImGui::Checkbox("提升进程优先级", &settings.boostProcessPriority))) {
        m_context.EnqueueEvent(events::gui::SetProcessPriority(settings.boostProcessPriority));
    }
    widgets::ExplanationTooltip("提升进程的优先级，可能有助于减少卡顿。",
                                m_context.displayScale);

    if (MakeDirty(ImGui::Checkbox("提升模拟器线程优先级", &settings.boostEmuThreadPriority))) {
        m_context.EnqueueEvent(events::emu::SetThreadPriority(settings.boostEmuThreadPriority));
    }
    widgets::ExplanationTooltip("提升模拟器线程的优先级，可能有助于减少抖动。",
                                m_context.displayScale);

    MakeDirty(ImGui::Checkbox("预加载光盘镜像到 RAM", &settings.preloadDiscImagesToRAM));
    widgets::ExplanationTooltip(
        "将整个光盘镜像预加载到内存。\n"
        "如果从慢速磁盘或网络加载镜像，可能有助于减少卡顿。",
        m_context.displayScale);

    MakeDirty(ImGui::Checkbox("记住上次加载的光盘镜像", &settings.rememberLastLoadedDisc));
    widgets::ExplanationTooltip(
        "启用后，Ymir 将在启动时自动加载最近使用的游戏光盘。",
        m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("行为");
    ImGui::PopFont();

    ImGui::TextUnformatted("模拟速度");
    widgets::ExplanationTooltip(
        "你可以随时调整并切换主要速度和备用速度。\n"
        "主要速度用于正常使用时的默认速度，而备用速度用作慢动作或限速快进选项，但你也可以随意使用。\n"
        "主要速度和备用速度分别默认重置为 100% 和 50%。",
        m_context.displayScale);

    if (ImGui::BeginTable("emu_speed", 3, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            if (MakeDirty(ImGui::RadioButton("主要##emu_speed", !settings.useAltSpeed))) {
                settings.useAltSpeed = false;
            }
        }
        if (ImGui::TableNextColumn()) {
            double speed = settings.mainSpeedFactor.Get() * 100.0;
            const double kMin = 10.0;
            const double kMax = 500.0;
            ImGui::SetNextItemWidth(300.0f * m_context.displayScale);
            if (MakeDirty(ImGui::SliderScalar("##main_emu_speed", ImGuiDataType_Double, &speed, &kMin, &kMax, "%.0lf%%",
                                              ImGuiSliderFlags_AlwaysClamp))) {
                settings.mainSpeedFactor = std::clamp(util::RoundToMultiple(speed * 0.01, 0.1), 0.1, 5.0);
            }
        }
        if (ImGui::TableNextColumn()) {
            if (MakeDirty(ImGui::Button("重置##main_emu_speed"))) {
                settings.mainSpeedFactor = 1.0;
            }
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            if (MakeDirty(ImGui::RadioButton("备用##emu_speed", settings.useAltSpeed))) {
                settings.useAltSpeed = true;
            }
        }
        if (ImGui::TableNextColumn()) {
            double speed = settings.altSpeedFactor.Get() * 100.0;
            const double kMin = 10.0;
            const double kMax = 500.0;
            ImGui::SetNextItemWidth(300.0f * m_context.displayScale);
            if (MakeDirty(ImGui::SliderScalar("##alternate_emu_speed", ImGuiDataType_Double, &speed, &kMin, &kMax,
                                              "%.0lf%%", ImGuiSliderFlags_AlwaysClamp))) {
                settings.altSpeedFactor = std::clamp(util::RoundToMultiple(speed * 0.01, 0.1), 0.1, 5.0);
            }
        }
        if (ImGui::TableNextColumn()) {
            if (MakeDirty(ImGui::Button("重置##alt_emu_speed"))) {
                settings.altSpeedFactor = 0.5;
            }
        }

        ImGui::EndTable();
    }

    MakeDirty(ImGui::Checkbox("失去焦点时暂停", &settings.pauseWhenUnfocused));
    widgets::ExplanationTooltip(
        "窗口失去焦点时模拟器将暂停，重新获得焦点时恢复。\n"
        "不影响手动暂停的行为 - 手动暂停会持续到焦点变化之后。",
        m_context.displayScale);

    MakeDirty(ImGui::Checkbox("加载光盘后取消暂停", &settings.unpauseOnDiscLoad));
    widgets::ExplanationTooltip("加载游戏光盘时模拟器将取消暂停。", m_context.displayScale);

    MakeDirty(ImGui::Checkbox("启动时暂停", &settings.startPaused));
    widgets::ExplanationTooltip("Ymir 启动时将以暂停状态启动模拟。", m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("更新");
    ImGui::PopFont();

#if Ymir_ENABLE_UPDATE_CHECKS
    MakeDirty(ImGui::Checkbox("启动时检查更新", &settings.checkForUpdates));
#endif
    MakeDirty(ImGui::Checkbox("更新到 nightly 构建", &settings.includeNightlyBuilds));
    widgets::ExplanationTooltip("启用后，当有新的 nightly 构建可用时，Ymir 也会通知你。",
                                m_context.displayScale);
    if (ImGui::Button("立即检查")) {
        m_context.EnqueueEvent(events::gui::CheckForUpdates());
    }
    ImGui::TextUnformatted("最新版本：");
    if (ImGui::BeginTable("updates", 3, ImGuiTableFlags_SizingFixedFit)) {
        std::unique_lock lock{m_context.locks.updates};
        auto version = [&](const char *name, const std::optional<UpdateInfo> &ver) {
            ImGui::PushID(name);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", name);

            ImGui::TableNextColumn();
            if (ver) {
                if (m_context.updates.inProgress) {
                    ImGui::TextUnformatted("检查中...");
                } else {
                    ImGui::Text("%s", ver->version.to_string().c_str());
                }
                ImGui::TableNextColumn();
                if (!ver->releaseNotesURL.empty()) {
                    if (ImGui::SmallButton("发行说明")) {
                        SDL_OpenURL(ver->releaseNotesURL.c_str());
                    }
                }
                if (!ver->downloadURL.empty()) {
                    if (!ver->releaseNotesURL.empty()) {
                        ImGui::SameLine();
                    }
                    if (ImGui::SmallButton("下载")) {
                        SDL_OpenURL(ver->downloadURL.c_str());
                    }
                }
            } else {
                ImGui::TextUnformatted("未检查");
            }

            ImGui::PopID();
        };
        version("Stable", m_context.updates.latestStable);
        version("Nightly", m_context.updates.latestNightly);

        ImGui::EndTable();
    }
    {
        std::unique_lock lock{m_context.locks.targetUpdate};
        if (m_context.targetUpdate) {
            ImGui::Text("有可更新到 v%s（%s 频道）的版本。",
                        m_context.targetUpdate->info.version.to_string().c_str(),
                        (m_context.targetUpdate->channel == ReleaseChannel::Stable ? "stable" : "nightly"));
        } else {
            ImGui::TextUnformatted("你已在使用最新版本。");
        }
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("截图");
    ImGui::PopFont();

    MakeDirty(
        ImGui::SliderInt("截图缩放", &settings.screenshotScale, 1, 4, "%u", ImGuiSliderFlags_AlwaysClamp));
    widgets::ExplanationTooltip("调整截图保存的缩放比例。\n"
                                "模拟器截取的截图没有宽高比失真，并使用最近邻插值进行缩放，以保留原始帧缓冲数据。",
                                m_context.displayScale);

    const std::filesystem::path screenshotsPath = m_context.profile.GetPath(ProfilePath::Screenshots);
    ImGui::TextUnformatted("截图保存到 ");
    ImGui::SameLine(0, 0);
    if (ImGui::TextLink(fmt::format("{}", screenshotsPath).c_str())) {
        SDL_OpenURL(fmt::format("file:///{}", screenshotsPath).c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("回退缓冲区");
    ImGui::PopFont();

    if (MakeDirty(ImGui::Checkbox("启用回退缓冲区", &settings.enableRewindBuffer))) {
        m_context.EnqueueEvent(events::gui::EnableRewindBuffer(settings.enableRewindBuffer));
    }
    widgets::ExplanationTooltip("允许你回退到之前的状态。\n"
                                "会增加内存使用并略微降低性能。",
                                m_context.displayScale);

    // TODO: rewind buffer size

    if (MakeDirty(ImGui::SliderInt("压缩级别", &settings.rewindCompressionLevel, 0, 16, "%d",
                                   ImGuiSliderFlags_AlwaysClamp))) {
        m_context.rewindBuffer.LZ4Accel = 1 << (16 - settings.rewindCompressionLevel);
    }
    widgets::ExplanationTooltip("调整压缩率与速度的平衡。\n"
                                "较高的值可提高压缩率，减少内存使用。\n"
                                "较低的值可提高压缩速度并改善模拟性能。",
                                m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("配置文件路径");
    ImGui::PopFont();

    const std::filesystem::path profileRoot = m_context.profile.GetPath(ProfilePath::Root);
    ImGui::TextUnformatted("当前配置文件位于 ");
    ImGui::SameLine(0, 0);
    if (ImGui::TextLink(fmt::format("{}", profileRoot).c_str())) {
        SDL_OpenURL(fmt::format("file:///{}", profileRoot).c_str());
    }

    ImGui::TextUnformatted("覆盖配置文件路径");

    if (ImGui::BeginTable("path_overrides", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("路径", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        auto drawRow = [&](const char *name, ProfilePath profPath) {
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(name);
            }
            if (ImGui::TableNextColumn()) {
                std::string label = fmt::format("##prof_path_override_{}", static_cast<uint32>(profPath));
                std::string imagePath = fmt::format("{}", profile.GetPathOverride(profPath));
                std::string currPath = fmt::format("{}", profile.GetPath(profPath));

                ImGui::SetNextItemWidth(
                    -(fileSelectorButtonWidth + clearButtonWidth + openButtonWidth + itemSpacingWidth * 3));
                if (MakeDirty(ImGui::InputTextWithHint(label.c_str(), currPath.c_str(), &imagePath,
                                                       ImGuiInputTextFlags_ElideLeft))) {
                    profile.SetPathOverride(profPath, std::u8string{imagePath.begin(), imagePath.end()});
                }
                ImGui::SameLine();
                if (ImGui::Button(fmt::format("...{}", label).c_str())) {
                    m_selectedProfPath = profPath;
                    m_context.EnqueueEvent(events::gui::SelectFolder({
                        .dialogTitle = fmt::format("选择 {} 目录", name),
                        .defaultPath = m_context.profile.GetPath(profPath),
                        .userdata = this,
                        .callback =
                            util::WrapSingleSelectionCallback<&GeneralSettingsView::ProcessPathOverrideSelection,
                                                              &util::NoopCancelFileDialogCallback,
                                                              &GeneralSettingsView::ProcessPathOverrideSelectionError>,
                    }));
                }
                ImGui::SameLine();
                if (ImGui::Button(fmt::format("清除{}", label).c_str())) {
                    profile.ClearOverridePath(profPath);
                }
                ImGui::SameLine();
                if (ImGui::Button(fmt::format("打开{}", label).c_str())) {
                    auto path = m_context.profile.GetPath(profPath);
                    SDL_OpenURL(fmt::format("file:///{}", path).c_str());
                }
            }
        };

        drawRow("IPL ROM 镜像", ProfilePath::IPLROMImages);
        drawRow("CD Block ROM 镜像", ProfilePath::CDBlockROMImages);
        drawRow("卡带 ROM 镜像", ProfilePath::ROMCartImages);
        drawRow("备份内存", ProfilePath::BackupMemory);
        drawRow("导出的备份文件", ProfilePath::ExportedBackups);
        drawRow("持久状态", ProfilePath::PersistentState);
        drawRow("存档状态", ProfilePath::SaveStates);
        drawRow("转储", ProfilePath::Dumps);
        drawRow("截图", ProfilePath::Screenshots);

        ImGui::EndTable();
    }
}

void GeneralSettingsView::ProcessPathOverrideSelection(void *userdata, std::filesystem::path file, int filter) {
    static_cast<GeneralSettingsView *>(userdata)->SelectPathOverride(file);
}

void GeneralSettingsView::ProcessPathOverrideSelectionError(void *userdata, const char *message, int filter) {
    static_cast<GeneralSettingsView *>(userdata)->ShowPathOverrideSelectionError(message);
}

void GeneralSettingsView::SelectPathOverride(std::filesystem::path file) {
    if (std::filesystem::is_directory(file)) {
        m_context.profile.SetPathOverride(m_selectedProfPath, file);
        MakeDirty();
    }
}

void GeneralSettingsView::ShowPathOverrideSelectionError(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(fmt::format("无法打开目录：{}", message)));
}

} // namespace app::ui
