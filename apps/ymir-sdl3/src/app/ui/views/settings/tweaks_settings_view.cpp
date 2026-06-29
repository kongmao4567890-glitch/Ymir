#include "tweaks_settings_view.hpp"

#include <app/ui/widgets/settings_widgets.hpp>

#include <app/events/emu_event_factory.hpp>

#include <misc/cpp/imgui_stdlib.h>

#include <SDL3/SDL_clipboard.h>

namespace app::ui {

TweaksSettingsView::TweaksSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void TweaksSettingsView::Display() {
    const float availWidth = ImGui::GetContentRegionAvail().x;

    ImGui::PushTextWrapPos(availWidth);
    ImGui::TextUnformatted("此标签页中列出的选项会影响模拟精度。\n"
                           "如果你在运行某些游戏时遇到问题，请尝试使用下面的推荐或最高质量/精度/兼容性预设。\n"
                           "性能预设可能会导致某些游戏出现问题。\n"
                           "报告问题时，请确保包含此列表：");
    ImGui::PopTextWrapPos();

    std::string tweaksList{};
    {
        auto checkbox = [](const char *name, bool value) { return fmt::format("[{}] {}", (value ? 'x' : ' '), name); };

        auto &settings = GetSettings();

        auto &enhancements = settings.video.enhancements;
        auto &swRenderer = settings.video.swRenderer;

        fmt::memory_buffer buf{};
        auto inserter = std::back_inserter(buf);

        // =============================================================================================================

        fmt::format_to(inserter, "## 增强\n");

        // -------------------------------------------------------------------------------------------------------------
        // Video

        fmt::format_to(inserter, "### 视频\n");
        fmt::format_to(inserter, "- {}\n", checkbox("去交错", enhancements.deinterlace));
        fmt::format_to(inserter, "- {}\n", checkbox("透明网格", enhancements.transparentMeshes));

        // =============================================================================================================

        fmt::format_to(inserter, "## 精度设置\n");

        // -------------------------------------------------------------------------------------------------------------
        // SH-2

        fmt::format_to(inserter, "### SH-2\n");
        fmt::format_to(inserter, "- {}\n", checkbox("模拟 SH-2 缓存", settings.system.emulateSH2Cache));
        fmt::format_to(inserter, "- SH-2 时钟因子：{}%\n", settings.system.sh2ClockFactor.Get());

        // -------------------------------------------------------------------------------------------------------------
        // Audio

        auto interpMode = [](ymir::core::config::audio::SampleInterpolationMode mode) {
            using enum ymir::core::config::audio::SampleInterpolationMode;
            switch (mode) {
            case NearestNeighbor: return "最近邻";
            case Linear: return "线性";
            default: return "（无效设置）";
            }
        };

        fmt::format_to(inserter, "### 音频\n");
        fmt::format_to(inserter, "- 插值模式：{}\n", interpMode(settings.audio.interpolation.Get()));
        fmt::format_to(inserter, "- 模拟步长粒度：{}\n",
                       widgets::settings::audio::StepGranularityToString(settings.audio.stepGranularity.Get()));

        // -------------------------------------------------------------------------------------------------------------
        // CD Block

        fmt::format_to(inserter, "### CD Block\n");
        fmt::format_to(inserter, "- {}\n", checkbox("使用底层 CD Block 模拟", settings.cdblock.useLLE.Get()));
        fmt::format_to(inserter, "- CD 读取速度：{}x\n", settings.cdblock.readSpeedFactor.Get());

        // =============================================================================================================

        fmt::format_to(inserter, "## 性能设置\n");

        // -------------------------------------------------------------------------------------------------------------
        // Video

        fmt::format_to(inserter, "### 视频\n");
        fmt::format_to(inserter, "- {}\n", checkbox("多线程 VDP1 渲染", swRenderer.threadedVDP1.Get()));
        fmt::format_to(inserter, "- {}\n", checkbox("多线程 VDP2 渲染", swRenderer.threadedVDP2.Get()));
        fmt::format_to(
            inserter, "  - {}\n",
            checkbox("为去交错渲染使用专用线程", swRenderer.threadedDeinterlacer.Get()));

        // -------------------------------------------------------------------------------------------------------------
        // Audio

        fmt::format_to(inserter, "### 音频\n");
        fmt::format_to(inserter, "- {}\n", checkbox("多线程 SCSP 和声音 CPU", settings.audio.threadedSCSP.Get()));

        // =============================================================================================================

        tweaksList = fmt::to_string(buf);
    }

    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    ImGui::InputTextMultiline("##tweaks_list", &tweaksList, ImVec2(availWidth, 0),
                              ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
    ImGui::PopFont();
    if (ImGui::Button("复制到剪贴板")) {
        SDL_SetClipboardText(tweaksList.c_str());
    }

    DisplayEnhancements();
    DisplayAccuracyOptions();
    DisplayPerformanceOptions();
}

void TweaksSettingsView::DisplayEnhancements() {
    auto &settings = GetSettings();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.xlarge);
    ImGui::SeparatorText("增强");
    ImGui::PopFont();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("预设：");
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("推荐##enhancements"))) {
        settings.video.enhancements.deinterlace = false;
        settings.video.enhancements.transparentMeshes = true;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted(
            "在质量和性能之间取得良好的平衡，且不影响兼容性。");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("最高质量##enhancements"))) {
        settings.video.enhancements.deinterlace = true;
        settings.video.enhancements.transparentMeshes = true;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("最大化质量，不考虑性能。");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("最佳性能##enhancements"))) {
        settings.video.enhancements.deinterlace = false;
        settings.video.enhancements.transparentMeshes = false;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("最大化性能，不考虑质量。");
        ImGui::EndTooltip();
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("视频");
    ImGui::PopFont();

    widgets::settings::video::enhancements::Deinterlace(m_context);
    widgets::settings::video::enhancements::TransparentMeshes(m_context);
}

void TweaksSettingsView::DisplayAccuracyOptions() {
    auto &settings = GetSettings();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.xlarge);
    ImGui::SeparatorText("精度");
    ImGui::PopFont();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("预设：");
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("推荐##accuracy"))) {
        m_context.EnqueueEvent(events::emu::SetEmulateSH2Cache(false));

        settings.system.emulateSH2Cache = false;
        settings.system.sh2ClockFactor = config_defaults::system::kDefaultSH2ClockFactor;

        settings.audio.interpolation = ymir::core::config::audio::SampleInterpolationMode::Linear;
        settings.audio.stepGranularity = 0;

        settings.cdblock.readSpeedFactor = 2;
        settings.cdblock.useLLE = false;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted(
            "在精度和性能之间取得良好的平衡，且不影响兼容性。");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("最高精度##accuracy"))) {
        m_context.EnqueueEvent(events::emu::SetEmulateSH2Cache(true));

        settings.system.emulateSH2Cache = true;
        settings.system.sh2ClockFactor = config_defaults::system::kDefaultSH2ClockFactor;

        settings.audio.interpolation = ymir::core::config::audio::SampleInterpolationMode::Linear;
        settings.audio.stepGranularity = 5;

        const bool hasCDBlockROMs = [&] {
            std::unique_lock lock{m_context.locks.romManager};
            return !m_context.romManager.GetCDBlockROMs().empty();
        }();
        settings.cdblock.readSpeedFactor = 2;
        settings.cdblock.useLLE = hasCDBlockROMs;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("最大化精度，不考虑性能。");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("最佳性能##accuracy"))) {
        m_context.EnqueueEvent(events::emu::SetEmulateSH2Cache(false));

        settings.system.emulateSH2Cache = false;
        settings.system.sh2ClockFactor = config_defaults::system::kDefaultSH2ClockFactor;

        settings.audio.interpolation = ymir::core::config::audio::SampleInterpolationMode::Linear;
        settings.audio.stepGranularity = 0;

        settings.cdblock.readSpeedFactor = 200;
        settings.cdblock.useLLE = false;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("最大化性能，不考虑精度。\n"
                               "降低某些游戏的兼容性。");
        ImGui::EndTooltip();
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("SH-2");
    ImGui::PopFont();

    widgets::settings::system::EmulateSH2Cache(m_context);
    widgets::settings::system::SH2ClockFactor(m_context);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("音频");
    ImGui::PopFont();

    widgets::settings::audio::InterpolationMode(m_context);
    widgets::settings::audio::StepGranularity(m_context);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("CD Block");
    ImGui::PopFont();

    widgets::settings::cdblock::CDBlockLLE(m_context);
    widgets::settings::cdblock::CDReadSpeed(m_context);
}

void TweaksSettingsView::DisplayPerformanceOptions() {
    auto &settings = GetSettings();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.xlarge);
    ImGui::SeparatorText("性能");
    ImGui::PopFont();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("预设：");
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("推荐##performance"))) {
        m_context.EnqueueEvent(events::emu::EnableThreadedVDP1(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedVDP2(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedDeinterlacer(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedSCSP(false));
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("在兼容性和性能之间取得良好的平衡。");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("最佳兼容性##performance"))) {
        m_context.EnqueueEvent(events::emu::EnableThreadedVDP1(false));
        m_context.EnqueueEvent(events::emu::EnableThreadedVDP2(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedDeinterlacer(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedSCSP(false));
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("最大化兼容性，不考虑性能。");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("最佳性能##performance"))) {
        m_context.EnqueueEvent(events::emu::EnableThreadedVDP1(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedVDP2(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedDeinterlacer(true));
        m_context.EnqueueEvent(events::emu::EnableThreadedSCSP(true));
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("最大化性能，不考虑精度。\n"
                               "降低某些游戏的兼容性。");
        ImGui::EndTooltip();
    }

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("视频");
    ImGui::PopFont();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
    ImGui::SeparatorText("软件渲染器");
    ImGui::PopFont();

    widgets::settings::video::swrenderer::ThreadedVDP(m_context);

    // TODO: hardware renderer options

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("音频");
    ImGui::PopFont();

    widgets::settings::audio::ThreadedSCSP(m_context);
}

} // namespace app::ui
