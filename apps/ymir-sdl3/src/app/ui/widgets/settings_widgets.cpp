#include "settings_widgets.hpp"

#include "common_widgets.hpp"

#include <app/settings.hpp>

#include <app/events/emu_event_factory.hpp>
#include <app/events/gui_event_factory.hpp>

#include <ymir/db/game_db.hpp>

#include <ymir/hw/cdblock/cdblock.hpp>

#include <fmt/format.h>

using namespace ymir;

namespace app::ui::widgets {

namespace settings::system {

    void EmulateSH2Cache(SharedContext &ctx) {
        const db::GameInfo *gameInfo = nullptr;
        {
            std::unique_lock lock{ctx.locks.disc};
            const auto &disc = ctx.saturn.GetDisc();
            if (!disc.sessions.empty()) {
                gameInfo = db::GetGameInfo(disc.header.productNumber, ctx.saturn.GetDiscHash());
            }
        }
        const bool forced =
            gameInfo != nullptr && BitmaskEnum(gameInfo->flags).AnyOf(db::GameInfo::Flags::ForceSH2Cache);

        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        bool emulateSH2Cache = settings.system.emulateSH2Cache || forced;
        if (forced) {
            ImGui::BeginDisabled();
        }
        if (settings.MakeDirty(ImGui::Checkbox("模拟 SH-2 缓存", &emulateSH2Cache))) {
            ctx.EnqueueEvent(events::emu::SetEmulateSH2Cache(emulateSH2Cache));
            settings.system.emulateSH2Cache = emulateSH2Cache;
        }
        widgets::ExplanationTooltip("启用 SH-2 缓存模拟。\n"
                                    "少数游戏需要此功能才能正常运行。\n"
                                    "模拟性能会降低约 10%。\n\n"
                                    "启用此选项后，两个 SH-2 CPU 的缓存都会被刷新。",
                                    ctx.displayScale);
        if (forced) {
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextColored(ctx.colors.notice, "由当前加载的游戏强制启用");
        }
    }

    void SH2ClockFactor(SharedContext &ctx) {
        const float paddingWidth = ImGui::GetStyle().FramePadding.x;
        const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
        const float resetButtonWidth = ImGui::CalcTextSize("重置").x + paddingWidth * 2;

        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        int factor = settings.system.sh2ClockFactor.Get();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("SH-2 时钟系数");
        widgets::ExplanationTooltip("警告：可能导致游戏崩溃、音频不同步或其他问题。\n"
                                    "请谨慎使用！\n"
                                    "\n"
                                    "调整 SH-2 CPU 的周期频率。同时影响 SCU DSP 和 VDP1。\n"
                                    "\n"
                                    "高于 100% 的值可以减少 CPU 密集型游戏的卡顿。\n"
                                    "低于 100% 的值可以在较慢的宿主 CPU 上提升性能。",
                                    ctx.displayScale);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-(resetButtonWidth + itemSpacingWidth));
        if (settings.MakeDirty(ImGui::SliderInt(
                "##sh2_clock_factor", &factor, app::config_defaults::system::kMinSH2ClockFactor,
                app::config_defaults::system::kMaxSH2ClockFactor, "%d%%", ImGuiSliderFlags_AlwaysClamp))) {
            settings.system.sh2ClockFactor = factor;
        }
        ImGui::SameLine();
        if (settings.MakeDirty(ImGui::Button("重置##sh2_clock_factor"))) {
            settings.system.sh2ClockFactor = app::config_defaults::system::kDefaultSH2ClockFactor;
        }
    }

} // namespace settings::system

namespace settings::video {

    void GraphicsBackendCombo(SharedContext &ctx) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        auto &videoSettings = settings.video;
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("图形后端：");
        widgets::ExplanationTooltip("选择用于渲染界面的图形 API。\n"
                                    //"Affects availability of additional features such as GPU rendering and shaders.\n"
                                    "\n"
                                    "更改会立即生效。如果新的图形后端初始化失败，"
                                    "此选项会自动恢复到上一个可用的后端选项。",
                                    ctx.displayScale);
        ImGui::SameLine();
        if (ImGui::BeginCombo("##graphics_backend", gfx::GraphicsBackendName(videoSettings.graphicsBackend),
                              ImGuiComboFlags_HeightLarge | ImGuiComboFlags_WidthFitPreview)) {
            auto item = [&](gfx::Backend backend) {
                if (settings.MakeDirty(ImGui::Selectable(gfx::GraphicsBackendName(backend),
                                                         videoSettings.graphicsBackend == backend))) {
                    ctx.EnqueueEvent(events::gui::SwitchGraphicsBackend(backend));
                }
            };
            for (gfx::Backend backend : gfx::kGraphicsBackends) {
                item(backend);
            }
            ImGui::EndCombo();
        }
    }

    void DisplayRotation(SharedContext &ctx, bool newLine) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        auto &videoSettings = settings.video;
        ImGui::PushID("##disp_rot");
        using Rot = Settings::Video::DisplayRotation;
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("屏幕旋转：");
        auto option = [&](const char *name, Rot value, bool sameLine = true) {
            if (sameLine) {
                ImGui::SameLine();
            }
            if (settings.MakeDirty(ImGui::RadioButton(name, videoSettings.rotation == value))) {
                videoSettings.rotation = value;
            }
        };
        option("正常", Rot::Normal, !newLine);
        option("90\u00B0 顺时针", Rot::_90CW);
        option("180\u00B0", Rot::_180);
        option("90\u00B0 逆时针", Rot::_90CCW);
        ImGui::PopID();
    }

    namespace swrenderer {

        void ThreadedVDP(SharedContext &ctx) {
            auto &settings = ctx.serviceLocator.GetRequired<Settings>();
            bool threadedVDP1 = settings.video.swRenderer.threadedVDP1;
            if (settings.MakeDirty(ImGui::Checkbox("多线程 VDP1 渲染器", &threadedVDP1))) {
                ctx.EnqueueEvent(events::emu::EnableThreadedVDP1(threadedVDP1));
            }
            widgets::ExplanationTooltip("在专用线程中运行软件 VDP1 渲染器。\n"
                                        "略微提升性能。\n"
                                        "禁用时，VDP1 渲染在模拟器线程上完成。",
                                        ctx.displayScale);

            bool threadedVDP2 = settings.video.swRenderer.threadedVDP2;
            if (settings.MakeDirty(ImGui::Checkbox("多线程 VDP2 渲染器", &threadedVDP2))) {
                ctx.EnqueueEvent(events::emu::EnableThreadedVDP2(threadedVDP2));
            }
            widgets::ExplanationTooltip(
                "在专用线程中运行软件 VDP2 渲染器。\n"
                "大幅提升性能，且似乎不会对游戏造成任何问题。\n"
                "禁用时，VDP2 渲染在模拟器线程上完成。\n"
                "\n"
                "强烈建议保持此选项启用，因为目前没有已知的缺点。",
                ctx.displayScale);

            ImGui::Indent();
            {
                if (!threadedVDP2) {
                    ImGui::BeginDisabled();
                }

                bool threadedDeinterlacer = settings.video.swRenderer.threadedDeinterlacer;
                if (settings.MakeDirty(
                        ImGui::Checkbox("为去隔行渲染使用独立线程", &threadedDeinterlacer))) {
                    ctx.EnqueueEvent(events::emu::EnableThreadedDeinterlacer(threadedDeinterlacer));
                }
                widgets::ExplanationTooltip(
                    "如果多线程 VDP2 渲染和去隔行增强均启用，则在专用线程上运行去隔行器。\n"
                    "在核心数充足的 CPU 上可显著提升增强功能的性能。\n"
                    "需要四核或更好的 CPU 才能获得最佳效果。\n"
                    "\n"
                    "如果你的 CPU 满足要求，强烈建议保持此选项启用。",
                    ctx.displayScale);

                if (!threadedVDP2) {
                    ImGui::EndDisabled();
                }
            }
            ImGui::Unindent();
        }

    } // namespace swrenderer

    namespace enhancements {

        void Deinterlace(SharedContext &ctx) {
            auto &settings = ctx.serviceLocator.GetRequired<Settings>();
            auto &videoSettings = settings.video;
            bool deinterlace = videoSettings.enhancements.deinterlace.Get();
            if (settings.MakeDirty(ImGui::Checkbox("视频去隔行", &deinterlace))) {
                videoSettings.enhancements.deinterlace = deinterlace;
            }
            widgets::ExplanationTooltip(
                "启用后，隔行高分辨率模式将以逐行模式渲染。\n"
                "在这些模式下启用时会明显影响性能。\n"
                "强烈建议同时启用“多线程 VDP2 渲染器”和“为去隔行渲染使用独立线程”选项以减轻性能影响。\n"
                "建议使用四核或更好的 CPU 来使用此选项。\n"
                "\n"
                "极少数游戏在启用此选项时可能出现图形伪影。目前已知的情况有：\n"
                "- True Pinball 在屏幕顶部将棋盘的下半部分与上半部分交错显示\n"
                "- Shienryuu 和 Pro-Pinball: The Web 的画面抖动",
                ctx.displayScale);
        }

        void TransparentMeshes(SharedContext &ctx) {
            auto &settings = ctx.serviceLocator.GetRequired<Settings>();
            auto &videoSettings = settings.video;
            bool transparentMeshes = videoSettings.enhancements.transparentMeshes.Get();
            if (settings.MakeDirty(ImGui::Checkbox("透明网格", &transparentMeshes))) {
                videoSettings.enhancements.transparentMeshes = transparentMeshes;
            }
            widgets::ExplanationTooltip(
                "启用后，网格（棋盘格图案）将渲染为透明多边形。",
                ctx.displayScale);
        }

    } // namespace enhancements

} // namespace settings::video

namespace settings::audio {

    void InterpolationMode(SharedContext &ctx) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        auto &audioSettings = settings.audio;

        using InterpMode = ymir::core::config::audio::SampleInterpolationMode;

        auto interpOption = [&](const char *name, InterpMode mode) {
            const std::string label = fmt::format("{}##sample_interp", name);
            ImGui::SameLine();
            if (settings.MakeDirty(ImGui::RadioButton(label.c_str(), audioSettings.interpolation == mode))) {
                audioSettings.interpolation = mode;
            }
        };

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("插值：");
        widgets::ExplanationTooltip("- 最近邻：开销最低，声音更粗糙。\n"
                                    "- 线性：硬件精确，声音更柔和。（默认）",
                                    ctx.displayScale);
        interpOption("最近邻", InterpMode::NearestNeighbor);
        interpOption("线性", InterpMode::Linear);
    }

    std::string StepGranularityToString(uint32 stepGranularity) {
        const uint32 numSteps = 32u >> stepGranularity;
        return fmt::format("{} {}{}", numSteps, (numSteps != 1 ? "槽位" : "槽位"),
                           (numSteps == 32 ? "（1 采样）" : ""));
    }

    void StepGranularity(SharedContext &ctx) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        auto &audioSettings = settings.audio;
        int stepGranularity = audioSettings.stepGranularity;

        if (ImGui::BeginTable("scsp_step_granularity", 2, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("标签", ImGuiTableColumnFlags_WidthFixed, 200.0f * ctx.displayScale);
            ImGui::TableSetupColumn("值", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("模拟步长粒度");
                widgets::ExplanationTooltip(
                    "警告：此设置非常消耗性能！\n"
                    "\n"
                    "增大此设置会使 SCSP 以更小的时间片进行模拟（最高可达采样级处理的 32 倍频率），"
                    "在换取更高精度的同时显著降低性能，而这种精度对绝大多数商业游戏并无益处。\n"
                    "\n"
                    "极少数游戏需要调整此设置。在大多数情况下建议保持为 0。\n"
                    "\n"
                    "此选项可能对需要额外精度的自制软件开发者有帮助。",
                    ctx.displayScale);
            }
            if (ImGui::TableNextColumn()) {
                ImGui::SetNextItemWidth(-1.0f);
                if (settings.MakeDirty(ImGui::SliderInt("##scsp_step_granularity", &stepGranularity, 0, 5, "%d",
                                                        ImGuiSliderFlags_AlwaysClamp))) {
                    audioSettings.stepGranularity = stepGranularity;
                }
            }
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("步长大小：%s", StepGranularityToString(stepGranularity).c_str());
                widgets::ExplanationTooltip(
                    "整个条形代表一个采样。SCSP 每个采样处理 32 个槽位，由条形中的分隔表示。\n\n"
                    "不同颜色的区域表示在当前粒度设置下，每个步长模拟采样的哪些部分。"
                    "粒度越高，SCSP 与其他组件之间的同步越紧密（精度更高），"
                    "但由于额外的上下文切换，性能会降低。",
                    ctx.displayScale);
            }
            if (ImGui::TableNextColumn()) {
                static constexpr ImU32 kGraphBackgroundColor = 0xAA253840;
                static constexpr ImU32 kGraphSliceFillColor = 0xE04AC3F7;
                static constexpr ImU32 kGraphSliceFillColorAlt = 0xE02193C4;
                static constexpr ImU32 kGraphSlotSeparatorColor = 0xE02A6F8C;

                const auto initBasePos = ImGui::GetCursorScreenPos();
                const auto totalAvail = ImGui::GetContentRegionAvail();
                const auto cellPadding = ImGui::GetStyle().CellPadding;
                const auto basePos = ImVec2(initBasePos.x, initBasePos.y + cellPadding.y);
                const auto avail = ImVec2(totalAvail.x, totalAvail.y - cellPadding.y * 2.0f);
                const float graphWidth = avail.x;
                const float graphHeight = ImGui::GetFrameHeight();
                const float sliceWidth = graphWidth / (1 << stepGranularity);
                const float slotWidth = graphWidth / 32.0f;
                const float sepThickness = 1.5f * ctx.displayScale;

                auto *drawList = ImGui::GetWindowDrawList();

                drawList->AddRectFilled(basePos, ImVec2(basePos.x + graphWidth, basePos.y + graphHeight),
                                        kGraphBackgroundColor);
                for (uint32 i = 0; i < (1 << stepGranularity); ++i) {
                    const float xStart = basePos.x + i * sliceWidth;
                    const float xEnd = xStart + sliceWidth;
                    drawList->AddRectFilled(ImVec2(xStart, basePos.y), ImVec2(xEnd, basePos.y + graphHeight),
                                            (i & 1) ? kGraphSliceFillColorAlt : kGraphSliceFillColor);
                }
                for (uint32 i = 1; i < 32; ++i) {
                    const float x = basePos.x + i * slotWidth;
                    drawList->AddLine(ImVec2(x, basePos.y), ImVec2(x, basePos.y + graphHeight),
                                      kGraphSlotSeparatorColor, sepThickness);
                }

                ImGui::Dummy(ImVec2(graphWidth, graphHeight));
            }

            ImGui::EndTable();
        }
    }

    void ThreadedSCSP(SharedContext &ctx) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        bool threadedSCSP = settings.audio.threadedSCSP;
        if (settings.MakeDirty(ImGui::Checkbox("多线程 SCSP 和音频 CPU", &threadedSCSP))) {
            ctx.EnqueueEvent(events::emu::EnableThreadedSCSP(threadedSCSP));
        }
        widgets::ExplanationTooltip("在专用线程中运行 SCSP 和 MC68EC000。\n"
                                    "以牺牲精度为代价提升性能。\n"
                                    "少数游戏在启用此选项时可能无法正常运行。",
                                    ctx.displayScale);
    }

} // namespace settings::audio

namespace settings::cdblock {

    void CDReadSpeed(SharedContext &ctx) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        auto &cdblockSettings = settings.cdblock;

        if (settings.cdblock.useLLE) {
            ImGui::BeginDisabled();
        }
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("CD 读取速度");
        widgets::ExplanationTooltip("更改模拟 CD 驱动器的最大读取速度。\n"
                                    "默认值为 2x，与真实土星的 CD 驱动器速度一致。\n"
                                    "速度越高加载时间越短，但可能降低兼容性。\n"
                                    "\n"
                                    "使用低级别 CD block 模拟时此选项不可用。",
                                    ctx.displayScale);

        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        static constexpr uint8 kMinReadSpeed = 2u;
        static constexpr uint8 kMaxReadSpeed = 200u;
        uint8 readSpeed = cdblockSettings.readSpeedFactor;
        if (settings.MakeDirty(ImGui::SliderScalar("##read_speed", ImGuiDataType_U8, &readSpeed, &kMinReadSpeed,
                                                   &kMaxReadSpeed, "%ux", ImGuiSliderFlags_AlwaysClamp))) {
            cdblockSettings.readSpeedFactor = readSpeed;
        }
        if (settings.cdblock.useLLE) {
            ImGui::EndDisabled();
        }
    }

    void CDBlockLLE(SharedContext &ctx) {
        auto &settings = ctx.serviceLocator.GetRequired<Settings>();
        auto &cdblockSettings = settings.cdblock;

        bool useLLE = cdblockSettings.useLLE;

        bool hasROMs;
        {
            std::unique_lock lock{ctx.locks.romManager};
            hasROMs = !ctx.romManager.GetCDBlockROMs().empty();
        }
        if (!hasROMs) {
            ImGui::BeginDisabled();
        }
        if (settings.MakeDirty(ImGui::Checkbox("使用低级别 CD Block 模拟", &useLLE))) {
            cdblockSettings.useLLE = useLLE;
        }
        widgets::ExplanationTooltip("选择高级别或低级别 CD Block 模拟。\n"
                                    "高级别模拟速度更快，但兼容性较低。\n"
                                    "低级别模拟精确得多，但要求更高，且需要有效的 CD block ROM 镜像。\n"
                                    "\n"
                                    "更改此选项会导致硬重置。",
                                    ctx.displayScale);
        if (!hasROMs) {
            ImGui::EndDisabled();
            ImGui::TextColored(ctx.colors.warn, "未找到 CD Block ROM。无法启用低级别模拟。");
        }
    }

} // namespace settings::cdblock

} // namespace app::ui::widgets
