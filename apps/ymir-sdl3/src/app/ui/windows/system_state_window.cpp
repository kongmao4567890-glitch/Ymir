#include "system_state_window.hpp"

#include <ymir/sys/saturn.hpp>

#include <app/events/emu_event_factory.hpp>
#include <app/events/gui_event_factory.hpp>

#include <app/ui/widgets/cartridge_widgets.hpp>
#include <app/ui/widgets/common_widgets.hpp>
#include <app/ui/widgets/system_widgets.hpp>

#include <SDL3/SDL_clipboard.h>

#include <fmt/std.h>

#include <cinttypes>

using namespace ymir;

namespace app::ui {

inline constexpr float kWindowWidth = 350.0f;

SystemStateWindow::SystemStateWindow(SharedContext &context)
    : WindowBase(context) {

    m_windowConfig.name = "系统状态";
    m_windowConfig.flags = ImGuiWindowFlags_AlwaysAutoResize;
}

void SystemStateWindow::PrepareWindow() {
    /*ImGui::SetNextWindowSizeConstraints(ImVec2(kWindowWidth * m_context.displayScale, 0),
                                        ImVec2(kWindowWidth * m_context.displayScale, FLT_MAX));*/
}

void SystemStateWindow::DrawContents() {
    const auto &settings = m_context.serviceLocator.GetRequired<Settings>();

    ImGui::BeginGroup();

    ImGui::SeparatorText("SMPC 参数");
    DrawSMPCParameters();

    ImGui::SeparatorText("状态");
    DrawScreen();
    DrawRealTimeClock();
    DrawClocks();

    ImGui::SeparatorText("CD 驱动器");
    if (settings.cdblock.useLLE) {
        DrawCDDrive();
    } else {
        DrawCDBlock();
    }

    ImGui::SeparatorText("备份 RAM");
    DrawBackupMemory();

    ImGui::SeparatorText("卡带");
    DrawCartridge();

    ImGui::SeparatorText("外设");
    DrawPeripherals();

    ImGui::SeparatorText("操作");
    DrawActions();

    ImGui::EndGroup();
}

void SystemStateWindow::DrawSMPCParameters() {
    sys::ClockSpeed clockSpeed = m_context.saturn.instance->GetClockSpeed();

    if (ImGui::BeginTable("sys_params", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("时钟频率");
            widgets::ExplanationTooltip(
                "选择系统时钟频率的分频系数。\n"
                "由游戏自动调整。\n"
                "在真实的土星上，游戏必须使用更快的时钟设置才能使用 352 像素宽的分辨率模式。",
                m_context.displayScale);
        }
        if (ImGui::TableNextColumn()) {
            if (ImGui::RadioButton("慢速", clockSpeed == sys::ClockSpeed::_320)) {
                m_context.EnqueueEvent(events::emu::SetClockSpeed(sys::ClockSpeed::_320));
            }
            widgets::ExplanationTooltip("320 像素", m_context.displayScale);
            ImGui::SameLine();
            if (ImGui::RadioButton("快速", clockSpeed == sys::ClockSpeed::_352)) {
                m_context.EnqueueEvent(events::emu::SetClockSpeed(sys::ClockSpeed::_352));
            }
            widgets::ExplanationTooltip("352 像素", m_context.displayScale);
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("视频制式");
        }
        if (ImGui::TableNextColumn()) {
            ui::widgets::VideoStandardSelector(m_context);
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("区域");
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::BeginItemTooltip()) {
                ImGui::TextUnformatted("更改此选项将导致硬重置");
                ImGui::EndTooltip();
            }
        }
        if (ImGui::TableNextColumn()) {
            ui::widgets::RegionSelector(m_context);
        }

        ImGui::EndTable();
    }
}

void SystemStateWindow::DrawScreen() {
    auto &probe = m_context.saturn.instance->VDP.GetProbe();
    auto [width, height] = probe.GetResolution();
    auto interlaceMode = probe.GetInterlaceMode();

    static constexpr const char *kInterlaceNames[]{"逐行", "(无效)", "单密度隔行",
                                                   "双密度隔行"};

    ImGui::TextUnformatted("分辨率：");
    ImGui::SameLine();
    ImGui::Text("%ux%u %s", width, height, kInterlaceNames[static_cast<uint8>(interlaceMode)]);
}

void SystemStateWindow::DrawRealTimeClock() {
    const auto dt = m_context.saturn.instance->SMPC.GetProbe().GetRTCDateTime();

    static constexpr const char *kWeekdays[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

    ImGui::Text("当前日期/时间：%s %04u/%02u/%02u %02u:%02u:%02u", kWeekdays[dt.weekday], dt.year, dt.month, dt.day,
                dt.hour, dt.minute, dt.second);
    // TODO: make it adjustable, sync to host
}

void SystemStateWindow::DrawClocks() {
    if (ImGui::BeginTable("sys_clocks", 3, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("组件");
        ImGui::TableSetupColumn("时钟");
        ImGui::TableSetupColumn("比率");
        ImGui::TableHeadersRow();

        const Saturn &saturn = *m_context.saturn.instance;

        const sys::ClockRatios clockRatios = saturn.GetClockRatios();

        const double clockScale = (double)saturn.configuration.system.sh2ClockFactor.Get().AsDouble();
        const double baseMasterClock =
            ((double)clockRatios.masterClock * clockRatios.masterClockNum / clockRatios.masterClockDen / 1000000.0);
        const double masterClock = baseMasterClock * clockScale;

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("SH-2、SCU 和 VDP");
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%.5lf MHz", masterClock);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("1:1");
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("SCU DSP");
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%.5lf MHz", masterClock * 0.5);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("1:2");
        }

        // Account for double-resolution
        const bool doubleWidth = saturn.VDP.GetProbe().GetResolution().width >= 640;
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("像素时钟");
        }
        if (ImGui::TableNextColumn()) {
            const double factor = doubleWidth ? 0.5 : 0.25;
            ImGui::Text("%.5lf MHz", baseMasterClock * factor);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("1:%u", doubleWidth ? 2u : 4u);
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("SCSP");
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%.5lf MHz", masterClock * clockRatios.SCSPNum / clockRatios.SCSPDen);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%" PRIu64 ":%" PRIu64, clockRatios.SCSPNum, clockRatios.SCSPDen);
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("MC68EC000");
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%.5lf MHz", masterClock * clockRatios.SCSPNum / clockRatios.SCSPDen * 0.5);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%" PRIu64 ":%" PRIu64, clockRatios.SCSPNum, clockRatios.SCSPDen * 2u);
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("CD Block SH-1");
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%.5lf MHz", masterClock * clockRatios.CDBlockNum / clockRatios.CDBlockDen);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%" PRIu64 ":%" PRIu64, clockRatios.CDBlockNum, clockRatios.CDBlockDen);
        }

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            ImGui::TextUnformatted("SMPC MCU");
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%.5lf MHz", masterClock * clockRatios.SMPCNum / clockRatios.SMPCDen);
        }
        if (ImGui::TableNextColumn()) {
            ImGui::Text("%" PRIu64 ":%" PRIu64, clockRatios.SMPCNum, clockRatios.SMPCDen);
        }

        ImGui::EndTable();
    }
}

void SystemStateWindow::DrawCDBlock() {
    auto &probe = m_context.saturn.instance->CDBlock.GetProbe();

    const uint8 status = probe.GetCurrentStatusCode();

    if (ImGui::Button(m_context.saturn.instance->CDBlock.IsTrayOpen() ? "关闭托盘" : "打开托盘")) {
        m_context.EnqueueEvent(events::emu::OpenCloseTray());
    }
    ImGui::SameLine();
    if (ImGui::Button("加载光盘...")) {
        m_context.EnqueueEvent(events::gui::LoadDisc());
    }
    ImGui::SameLine();
    if (ImGui::Button("弹出光盘")) {
        m_context.EnqueueEvent(events::emu::EjectDisc());
    }

    DrawDiscImage();

    switch (status) {
    case cdblock::kStatusCodeBusy: ImGui::TextUnformatted("忙碌"); break;
    case cdblock::kStatusCodePause: ImGui::TextUnformatted("已暂停"); break;
    case cdblock::kStatusCodeStandby: ImGui::TextUnformatted("待机"); break;
    case cdblock::kStatusCodePlay:
        ImGui::Text("正在播放轨道 %u，索引 %u（%s）", probe.GetCurrentTrack(), probe.GetCurrentIndex(),
                    (probe.GetCurrentControlADRBits() == 0x01 ? "CDDA" : "数据"));
        break;
    case cdblock::kStatusCodeSeek: ImGui::TextUnformatted("寻道中"); break;
    case cdblock::kStatusCodeScan:
        ImGui::Text("正在扫描轨道 %u，索引 %u（%s）", probe.GetCurrentTrack(), probe.GetCurrentIndex(),
                    (probe.GetCurrentControlADRBits() == 0x01 ? "CDDA" : "数据"));
        break;
    case cdblock::kStatusCodeOpen: ImGui::TextUnformatted("托盘已打开"); break;
    case cdblock::kStatusCodeNoDisc: ImGui::TextUnformatted("无光盘"); break;
    case cdblock::kStatusCodeRetry: ImGui::TextUnformatted("重试中"); break;
    case cdblock::kStatusCodeError: ImGui::TextUnformatted("错误"); break;
    case cdblock::kStatusCodeFatal: ImGui::TextUnformatted("严重错误"); break;
    }

    ImGui::Text("读取速度：%ux", probe.GetReadSpeed());

    const uint32 fad = probe.GetCurrentFrameAddress();
    const uint8 repeat = probe.GetCurrentRepeatCount();
    const uint8 maxRepeat = probe.GetMaxRepeatCount();
    const cdblock::MSF msf = cdblock::FADToMSF(fad);

    if (status == cdblock::kStatusCodePlay || status == cdblock::kStatusCodeScan) {
        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        if (msf.m == 0) {
            ImGui::TextDisabled("00");
        } else {
            if (msf.m < 10) {
                ImGui::TextDisabled("0");
                ImGui::SameLine(0, 0);
            }
            ImGui::Text("%u", msf.m);
        }
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(":");
        ImGui::SameLine(0, 0);
        if (msf.m == 0 && msf.s == 0) {
            ImGui::TextDisabled("00");
        } else {
            if (msf.m == 0 && msf.s < 10) {
                ImGui::TextDisabled("0");
                ImGui::SameLine(0, 0);
                ImGui::Text("%u", msf.s);
            } else {
                ImGui::Text("%02u", msf.s);
            }
        }
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(".");
        ImGui::SameLine(0, 0);
        if (msf.m == 0 && msf.s == 0 && msf.f == 0) {
            ImGui::TextDisabled("00");
        } else {
            if (msf.m == 0 && msf.s == 0 && msf.f < 10) {
                ImGui::TextDisabled("0");
                ImGui::SameLine(0, 0);
                ImGui::Text("%u", msf.f);
            } else {
                ImGui::Text("%02u", msf.f);
            }
        }
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("MM:SS.FF\n分、秒和帧\n（每秒 75 帧）");

        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" :: ");
        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        const uint32 numZeros = std::countl_zero(fad) / 4 - 2; // FAD is 24 bits
        ImGui::TextDisabled("%0*u", numZeros, 0);
        ImGui::SameLine(0, 0);
        ImGui::Text("%X", fad);
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("帧地址 (FAD)");
    } else {
        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        ImGui::TextDisabled("--");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(":");
        ImGui::SameLine(0, 0);
        ImGui::TextDisabled("--");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(".");
        ImGui::SameLine(0, 0);
        ImGui::TextDisabled("--");
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("MM:SS.FF\n分、秒和帧\n（每秒 75 帧）");

        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" :: ");
        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        ImGui::TextDisabled("------");
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("帧地址 (FAD)");
    }

    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted(" :: ");
    ImGui::SameLine(0, 0);

    if (maxRepeat == 0xF) {
        ImGui::TextUnformatted("无限循环");
    } else if (maxRepeat > 0) {
        ImGui::Text("重复 %u / %u", repeat, maxRepeat);
    } else {
        ImGui::TextUnformatted("不重复");
    }

    if (status == cdblock::kStatusCodePlay) {
        std::string file = probe.GetPathAtFrameAddress(fad);
        if (!file.empty()) {
            ImGui::Text("正在读取文件 %s", file.c_str());
        } else {
            ImGui::TextUnformatted("非文件");
        }
    } else {
        ImGui::TextUnformatted("");
    }
}

void SystemStateWindow::DrawCDDrive() {
    auto &probe = m_context.saturn.instance->CDDrive.GetProbe();

    using CDOp = cdblock::CDDrive::Operation;

    const auto &status = probe.GetStatus();

    if (ImGui::Button(status.operation == CDOp::TrayOpen ? "关闭托盘" : "打开托盘")) {
        m_context.EnqueueEvent(events::emu::OpenCloseTray());
    }
    ImGui::SameLine();
    if (ImGui::Button("加载光盘...")) {
        m_context.EnqueueEvent(events::gui::LoadDisc());
    }
    ImGui::SameLine();
    if (ImGui::Button("弹出光盘")) {
        m_context.EnqueueEvent(events::emu::EjectDisc());
    }

    DrawDiscImage();

    switch (status.operation) {
    case CDOp::Reset: ImGui::TextUnformatted("重置"); break;
    case CDOp::Idle: ImGui::TextUnformatted("空闲"); break;
    case CDOp::Stopped: ImGui::TextUnformatted("已停止"); break;
    case CDOp::TrayOpen: ImGui::TextUnformatted("托盘已打开"); break;
    case CDOp::NoDisc: ImGui::TextUnformatted("无光盘"); break;
    case CDOp::ReadTOC: ImGui::TextUnformatted("正在读取 TOC"); break;
    case CDOp::DiscChanged: ImGui::TextUnformatted("光盘已更换"); break;
    case CDOp::ReadDataSector:
        ImGui::Text("正在读取轨道 %u，索引 %u（数据）", status.trackNum, status.indexNum);
        break;
    case CDOp::ReadAudioSector:
        ImGui::Text("正在播放轨道 %u，索引 %u（CDDA）", status.trackNum, status.indexNum);
        break;
    case CDOp::ScanAudioSector:
        ImGui::Text("正在扫描轨道 %u，索引 %u（CDDA）", status.trackNum, status.indexNum);
        break;
    case CDOp::Seek: ImGui::TextUnformatted("寻道中"); break;
    case CDOp::SeekSecurityRingB2: [[fallthrough]];
    case CDOp::SeekSecurityRingB6: ImGui::TextUnformatted("正在寻道安全环"); break;
    default: ImGui::Text("未知 (%02X)", static_cast<uint8>(status.operation)); break;
    }

    ImGui::Text("读取速度：%ux", probe.GetReadSpeed());

    const bool isReading = status.operation == CDOp::ReadDataSector || status.operation == CDOp::ReadAudioSector ||
                           status.operation == CDOp::ScanAudioSector;
    const bool isSeeking = status.operation == CDOp::Seek || status.operation == CDOp::SeekSecurityRingB2 ||
                           status.operation == CDOp::SeekSecurityRingB6;

    const uint32 fad = isSeeking ? probe.GetTargetFrameAddress() : probe.GetCurrentFrameAddress();
    const cdblock::MSF msf = cdblock::FADToMSF(fad);

    if (isReading || isSeeking) {
        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        if (msf.m == 0) {
            ImGui::TextDisabled("00");
        } else {
            if (msf.m < 10) {
                ImGui::TextDisabled("0");
                ImGui::SameLine(0, 0);
            }
            ImGui::Text("%u", msf.m);
        }
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(":");
        ImGui::SameLine(0, 0);
        if (msf.m == 0 && msf.s == 0) {
            ImGui::TextDisabled("00");
        } else {
            if (msf.m == 0 && msf.s < 10) {
                ImGui::TextDisabled("0");
                ImGui::SameLine(0, 0);
                ImGui::Text("%u", msf.s);
            } else {
                ImGui::Text("%02u", msf.s);
            }
        }
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(".");
        ImGui::SameLine(0, 0);
        if (msf.m == 0 && msf.s == 0 && msf.f == 0) {
            ImGui::TextDisabled("00");
        } else {
            if (msf.m == 0 && msf.s == 0 && msf.f < 10) {
                ImGui::TextDisabled("0");
                ImGui::SameLine(0, 0);
                ImGui::Text("%u", msf.f);
            } else {
                ImGui::Text("%02u", msf.f);
            }
        }
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("MM:SS.FF\n分、秒和帧\n（每秒 75 帧）");

        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" :: ");
        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        const uint32 numZeros = std::countl_zero(fad) / 4 - 2; // FAD is 24 bits
        ImGui::TextDisabled("%0*u", numZeros, 0);
        ImGui::SameLine(0, 0);
        ImGui::Text("%X", fad);
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("帧地址 (FAD)");
    } else {
        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        ImGui::TextDisabled("--");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(":");
        ImGui::SameLine(0, 0);
        ImGui::TextDisabled("--");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(".");
        ImGui::SameLine(0, 0);
        ImGui::TextDisabled("--");
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("MM:SS.FF\n分、秒和帧\n（每秒 75 帧）");

        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" :: ");
        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
        ImGui::TextDisabled("------");
        ImGui::PopFont();
        ImGui::EndGroup();
        ImGui::SetItemTooltip("帧地址 (FAD)");
    }

    if (status.operation == CDOp::ReadDataSector) {
        std::string file = probe.GetPathAtFrameAddress(fad);
        if (!file.empty()) {
            ImGui::Text("正在读取文件 %s", file.c_str());
        } else {
            ImGui::TextUnformatted("非文件");
        }
    } else {
        ImGui::TextUnformatted("");
    }
}

void SystemStateWindow::DrawDiscImage() {
    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
    if (m_context.state.loadedDiscImagePath.empty()) {
        ImGui::TextUnformatted("未加载镜像");
    } else {
        ImGui::Text("镜像来自 %s", fmt::format("{}", m_context.state.loadedDiscImagePath).c_str());
        std::string hash{};
        std::string serial{};
        {
            std::unique_lock lock{m_context.locks.disc};
            hash = ToString(m_context.saturn.GetDiscHash());
            serial = m_context.saturn.GetDisc().header.productNumber;
        }

        auto draw = [&](const char *name, std::string_view value) {
            if (value.empty()) {
                ImGui::Text("%s：<空>", name);
            } else {
                ImGui::Text("%s：%s", name, value.data());
                ImGui::PushID(name);
                ImGui::SameLine();
                if (ImGui::SmallButton("复制")) {
                    SDL_SetClipboardText(value.data());
                }
                ImGui::PopID();
            }
        };

        draw("序列号", serial.c_str());
        draw("哈希", hash.c_str());
    }
    ImGui::PopTextWrapPos();
}

void SystemStateWindow::DrawBackupMemory() {
    if (ImGui::Button("打开备份 RAM 管理器")) {
        m_context.EnqueueEvent(events::gui::OpenBackupMemoryManager());
    }

    if (ImGui::BeginTable("bup_info", 3, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("设备");
        ImGui::TableSetupColumn("容量");
        ImGui::TableSetupColumn("已用块数");
        ImGui::TableHeadersRow();

        auto drawBup = [&](const char *name, bup::IBackupMemory *bup) {
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::TextUnformatted(name);
            }
            if (bup) {
                if (ImGui::TableNextColumn()) {
                    ImGui::Text("%u KiB", bup->Size() / 1024u);
                }
                if (ImGui::TableNextColumn()) {
                    if (bup->IsHeaderValid()) {
                        ImGui::Text("%u / %u", bup->GetUsedBlocks(), bup->GetTotalBlocks());
                    } else {
                        ImGui::TextUnformatted("无效");
                    }
                }
            } else {
                if (ImGui::TableNextColumn()) {
                    ImGui::TextUnformatted("-");
                }
                if (ImGui::TableNextColumn()) {
                    ImGui::TextUnformatted("-");
                }
            }
        };

        drawBup("内部", &m_context.saturn.instance->mem.GetInternalBackupRAM());
        {
            std::unique_lock lock{m_context.locks.cart};
            if (auto *bupCart = m_context.saturn.instance->GetCartridge().As<cart::CartType::BackupMemory>()) {
                drawBup("外部", &bupCart->GetBackupMemory());
            } else {
                drawBup("外部", nullptr);
            }
        }

        ImGui::EndTable();
    }
}

void SystemStateWindow::DrawCartridge() {
    ImGui::Button("插入...");
    if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        if (ImGui::MenuItem("备份 RAM")) {
            m_context.EnqueueEvent(events::gui::OpenBackupMemoryCartFileDialog());
        }
        if (ImGui::MenuItem("8 Mbit DRAM")) {
            m_context.EnqueueEvent(events::emu::Insert8MbitDRAMCartridge());
        }
        if (ImGui::MenuItem("32 Mbit DRAM")) {
            m_context.EnqueueEvent(events::emu::Insert32MbitDRAMCartridge());
        }
        if (ImGui::MenuItem("48 Mbit DRAM")) {
            m_context.EnqueueEvent(events::emu::Insert48MbitDRAMCartridge());
        }
        if (ImGui::MenuItem("16 Mbit ROM")) {
            m_context.EnqueueEvent(events::gui::OpenROMCartFileDialog());
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("移除")) {
        m_context.EnqueueEvent(events::emu::RemoveCartridge());
    }
    ImGui::SameLine();

    uint8 cartID;
    {
        std::unique_lock lock{m_context.locks.cart};
        auto &cart = m_context.saturn.instance->GetCartridge();
        cartID = cart.GetID();
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("[ID %02X] ", cartID);
    ImGui::SameLine(0, 0);
    widgets::CartridgeInfo(m_context);
}

void SystemStateWindow::DrawPeripherals() {
    if (ImGui::Button("配置##peripherals")) {
        m_context.EnqueueEvent(events::gui::OpenSettings(SettingsTab::Input));
    }

    if (ImGui::BeginTable("sys_peripherals", 2, ImGuiTableFlags_SizingFixedFit)) {
        auto &port1 = m_context.saturn.instance->SMPC.GetPeripheralPort1();
        auto type1 = port1.GetPeripheral().GetType();

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            // ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("端口 1：");
        }
        if (ImGui::TableNextColumn()) {
            // ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", peripheral::GetPeripheralName(type1).data());
        }
        /*if (ImGui::TableNextColumn()) {
            if (ImGui::Button("Configure...##port_1")) {
                // TODO: send GUI event to invoke peripheral configuration
            }
        }*/

        auto &port2 = m_context.saturn.instance->SMPC.GetPeripheralPort2();
        auto type2 = port2.GetPeripheral().GetType();

        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
            // ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("端口 2：");
        }
        if (ImGui::TableNextColumn()) {
            // ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", peripheral::GetPeripheralName(type2).data());
        }
        /*if (ImGui::TableNextColumn()) {
            if (ImGui::Button("Configure...##port_2")) {
                // TODO: send GUI event to invoke peripheral configuration
            }
        }*/

        ImGui::EndTable();
    }
}

void SystemStateWindow::DrawActions() {
    if (ImGui::Button("硬重置")) {
        m_context.EnqueueEvent(events::emu::HardReset());
    }
    ImGui::SameLine();
    if (ImGui::Button("软重置")) {
        m_context.EnqueueEvent(events::emu::SoftReset());
    }
    // TODO: Let's not make it that easy to accidentally wipe system settings
    /*ImGui::SameLine();
    if (ImGui::Button("Factory reset")) {
        m_context.EnqueueEvent(events::emu::FactoryReset());
    }*/
}

} // namespace app::ui
