#include "video_settings_view.hpp"

#include <app/events/gui_event_factory.hpp>

#include <app/ui/widgets/common_widgets.hpp>
#include <app/ui/widgets/settings_widgets.hpp>

namespace app::ui {

VideoSettingsView::VideoSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void VideoSettingsView::Display() {
    auto &settings = GetSettings().video;

    // -----------------------------------------------------------------------------------------------------------------

    /*ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("General");
    ImGui::PopFont();

    widgets::settings::video::GraphicsBackendCombo(m_context);*/

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("显示");
    ImGui::PopFont();

    MakeDirty(ImGui::Checkbox("强制整数缩放", &settings.forceIntegerScaling));
    MakeDirty(ImGui::Checkbox("强制宽高比", &settings.forceAspectRatio));
    widgets::ExplanationTooltip("禁用时，强制使用方形像素。", m_context.displayScale);
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("4:3"))) {
        settings.forcedAspect = 4.0 / 3.0;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("3:2"))) {
        settings.forcedAspect = 3.0 / 2.0;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("16:10"))) {
        settings.forcedAspect = 16.0 / 10.0;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::Button("16:9"))) {
        settings.forcedAspect = 16.0 / 9.0;
    }
    // TODO: aspect ratio selector? slider?

    widgets::settings::video::DisplayRotation(m_context);

    ImGui::Separator();

    MakeDirty(ImGui::Checkbox("自动调整窗口以适应屏幕", &settings.autoResizeWindow));
    widgets::ExplanationTooltip(
        "如果禁用了强制宽高比，则在显示分辨率改变时调整并重新居中窗口。",
        m_context.displayScale);
    ImGui::SameLine();
    if (settings.displayVideoOutputInWindow) {
        ImGui::BeginDisabled();
    }
    if (MakeDirty(ImGui::Button("立即调整"))) {
        m_context.EnqueueEvent(events::gui::FitWindowToScreen());
    }
    if (settings.displayVideoOutputInWindow) {
        ImGui::EndDisabled();
    }

    if (MakeDirty(ImGui::Checkbox("窗口化视频输出", &settings.displayVideoOutputInWindow))) {
        m_context.EnqueueEvent(events::gui::FitWindowToScreen());
    }
    widgets::ExplanationTooltip("将显示移动到专用窗口。\n"
                                "与调试器窗口一起使用时可能会有所帮助。",
                                m_context.displayScale);

    ImGui::Separator();

    bool fullScreen = settings.fullScreen.Get();
    if (MakeDirty(ImGui::Checkbox("全屏", &fullScreen))) {
        settings.fullScreen = fullScreen;
    }

    MakeDirty(ImGui::Checkbox("双击切换全屏", &settings.doubleClickToFullScreen));
    widgets::ExplanationTooltip("如果你正在使用 Virtua Gun 或 Shuttle Mouse，此选项将不起作用。",
                                m_context.displayScale);

    auto formatDisplay = [&](SDL_DisplayID id) -> std::string {
        if (m_context.display.list.contains(id)) {
            const auto &display = m_context.display.list.at(id);
            return fmt::format("{} [{}x{}]", display.name, display.bounds.x, display.bounds.y);
        }
        const SDL_DisplayID currDisplayID = SDL_GetDisplayForWindow(m_context.screen.window);
        SDL_Rect bounds{};
        if (SDL_GetDisplayBounds(currDisplayID, &bounds)) {
            return fmt::format("当前显示器 - {} [{}x{}]", SDL_GetDisplayName(currDisplayID), bounds.x, bounds.y);
        }
        return fmt::format("当前显示器 - {} [?x?]", SDL_GetDisplayName(currDisplayID));
    };

    auto formatMode = [&](const display::DisplayMode &mode) -> std::string {
        if (mode.IsValid()) {
            const auto *pixelFormat = SDL_GetPixelFormatDetails(mode.pixelFormat);
            return fmt::format("{}x{} {}bpp {} Hz", mode.width, mode.height, pixelFormat->bits_per_pixel,
                               mode.refreshRate);
        }
        const SDL_DisplayMode *desktopMode = SDL_GetDesktopDisplayMode(m_context.GetSelectedDisplay());
        const SDL_PixelFormatDetails *pixelFormat = SDL_GetPixelFormatDetails(desktopMode->format);
        return fmt::format("桌面分辨率 - {}x{} {}bpp {} Hz", desktopMode->w, desktopMode->h,
                           pixelFormat->bits_per_pixel, desktopMode->refresh_rate);
    };

    if (ImGui::BeginCombo("全屏显示器", formatDisplay(m_context.display.id).c_str())) {
        auto entry = [&](SDL_DisplayID id) {
            if (MakeDirty(ImGui::Selectable(formatDisplay(id).c_str(), m_context.display.id == id))) {
                if (m_context.display.id != id) {
                    m_context.display.id = id;

                    const char *displayName = SDL_GetDisplayName(id);
                    settings.fullScreenDisplay.name = displayName != nullptr ? displayName : "";
                    SDL_Rect bounds{};
                    if (SDL_GetDisplayBounds(id, &bounds)) {
                        settings.fullScreenDisplay.bounds.x = bounds.x;
                        settings.fullScreenDisplay.bounds.y = bounds.y;
                    } else {
                        settings.fullScreenDisplay.bounds.x = 0;
                        settings.fullScreenDisplay.bounds.y = 0;
                    }

                    // Revert to desktop resolution when switching displays
                    settings.fullScreenMode = {};

                    m_context.EnqueueEvent(events::gui::ApplyFullscreenMode());
                }
            }
        };

        entry(0);
        for (const auto &[id, _] : m_context.display.list) {
            entry(id);
        }
        ImGui::EndCombo();
    }
    widgets::ExplanationTooltip(
        "选择切换到全屏模式时要使用的显示器。\n"
        "\n"
        "[方括号] 中的数字表示显示器在多显示器系统中的虚拟位置。[0x0] 是你的主显示器。\n"
        "\n"
        "\"当前显示器\" 选项使 Ymir 在窗口所在的显示器上进入全屏。",
        m_context.displayScale);

    if (ImGui::BeginCombo("全屏分辨率",
                          settings.borderlessFullScreen ? "无边框全屏"
                                                        : formatMode(settings.fullScreenMode).c_str(),
                          ImGuiComboFlags_HeightLarge)) {
        auto entry = [&](const display::DisplayMode &mode) {
            if (MakeDirty(ImGui::Selectable(formatMode(mode).c_str(),
                                            !settings.borderlessFullScreen && settings.fullScreenMode == mode))) {
                if (settings.fullScreenMode != mode || settings.borderlessFullScreen) {
                    settings.borderlessFullScreen = false;
                    settings.fullScreenMode = mode;

                    m_context.EnqueueEvent(events::gui::ApplyFullscreenMode());
                }
            }
        };

        if (MakeDirty(ImGui::Selectable("无边框全屏", settings.borderlessFullScreen))) {
            if (!settings.borderlessFullScreen) {
                settings.borderlessFullScreen = true;
                settings.fullScreenMode = {};

                m_context.EnqueueEvent(events::gui::ApplyFullscreenMode());
            }
        }

        entry({});

        const SDL_DisplayID selectedDisplay =
            m_context.display.id != 0 ? m_context.display.id : SDL_GetDisplayForWindow(m_context.screen.window);
        const auto &info = m_context.display.list.at(selectedDisplay);
        for (const auto &mode : info.modes) {
            entry(mode);
        }
        ImGui::EndCombo();
    }
    widgets::ExplanationTooltip("选择切换到全屏模式时要使用的分辨率。\n"
                                "\n"
                                "除 \"无边框全屏\" 外的所有选项都是独占模式。\n"
                                "\n"
                                "当显示器被更改或移除，或在使用当前显示器并将窗口移动到不同显示器时，此选项会重置。",
                                m_context.displayScale);

    ImGui::Separator();

    MakeDirty(ImGui::Checkbox("窗口模式下同步视频", &settings.syncInWindowedMode));
    widgets::ExplanationTooltip(
        "启用后，在窗口模式下将界面更新与模拟器渲染同步。\n"
        "这极大地改善了帧节奏，但可能降低界面性能。",
        m_context.displayScale);

    MakeDirty(ImGui::Checkbox("全屏模式下同步视频", &settings.syncInFullscreenMode));
    widgets::ExplanationTooltip(
        "启用后，在全屏模式下将界面更新与模拟器渲染同步。\n"
        "这极大地改善了帧节奏，但可能降低界面性能。",
        m_context.displayScale);

    MakeDirty(
        ImGui::Checkbox("同步视频时使用完整刷新率", &settings.useFullRefreshRateWithVideoSync));
    widgets::ExplanationTooltip(
        "启用后，在同步视频时，界面帧率将调整为不超过显示器刷新率的模拟器目标帧率的最大整数倍。\n"
        "禁用时，界面帧率将限制为模拟器的目标帧率。\n"
        "启用此选项可以在高刷新率显示器上略微减少输入延迟。\n"
        "\n"
        "警告：在启用此选项之前，请禁用上面的 \"窗口/全屏模式下同步视频\" 选项，并检查报告的界面帧率是否与显示器的刷新率匹配。"
        "如果它被限制为低于显示器刷新率的任何值（例如在 120 Hz 显示器上为 60 fps），启用此选项将显著减慢模拟。",
        m_context.displayScale);

    MakeDirty(ImGui::Checkbox("在低刷新率显示器上减少视频延迟", &settings.reduceLatency));
    widgets::ExplanationTooltip(
        "此选项影响当模拟器生成的帧数多于显示器能够显示的帧数时显示哪一帧：\n"
        "- 启用时，显示最新渲染的帧。略微减少感知的输入延迟。\n"
        "- 禁用时，显示自上次刷新以来渲染的第一帧。通过跳过一些帧缓冲拷贝来略微提高整体模拟性能。\n"
        "\n"
        "如果显示器的刷新率高于模拟器的目标帧率，此选项无效。",
        m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("增强");
    ImGui::PopFont();

    widgets::settings::video::enhancements::Deinterlace(m_context);
    widgets::settings::video::enhancements::TransparentMeshes(m_context);

    // 渲染分辨率倍数
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("VDP1 渲染分辨率倍数");
    ImGui::SameLine();
    {
        auto &resolutionScale = settings.enhancements.resolutionScale;
        const float currentScale = resolutionScale.Get();
        const char *scaleLabels[] = {"1x (原始)", "2x", "3x", "4x"};
        const float scaleValues[] = {1.0f, 2.0f, 3.0f, 4.0f};
        const int numOptions = 4;
        int currentIndex = 0;
        for (int i = 0; i < numOptions; i++) {
            if (currentScale == scaleValues[i]) {
                currentIndex = i;
                break;
            }
        }
        ImGui::SetNextItemWidth(200 * m_context.displayScale);
        if (MakeDirty(ImGui::Combo("##ResolutionScale", &currentIndex, scaleLabels, numOptions))) {
            resolutionScale = scaleValues[currentIndex];
        }
    }
    widgets::ExplanationTooltip(
        "提高 VDP1 渲染分辨率倍数可以让 3D 多边形以更高分辨率光栅化，\n"
        "使多边形边缘更平滑、纹理更清晰，产生真正的超采样抗锯齿效果。\n"
        "\n"
        "- 1x: 原始分辨率 (512x256)，性能最佳\n"
        "- 2x: 2倍分辨率 (1024x512)，明显改善3D画面质量\n"
        "- 3x: 3倍分辨率 (1536x768)，高质量画面，需要较强CPU\n"
        "- 4x: 4倍分辨率 (2048x1024)，最高质量，需要高性能CPU\n"
        "\n"
        "此选项直接提升 VDP1 3D 渲染器的内部分辨率，而非简单的显示缩放。\n"
        "较高的倍数会显著增加 CPU 负载和内存使用。",
        m_context.displayScale);

    // -----------------------------------------------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::SeparatorText("软件渲染器");
    ImGui::PopFont();

    widgets::settings::video::swrenderer::ThreadedVDP(m_context);
}

} // namespace app::ui
