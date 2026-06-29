#include "scsp_slots_view.hpp"

#include <ymir/hw/scsp/scsp.hpp>

#include <app/ui/fonts/IconsMaterialSymbols.h>
#include <app/ui/widgets/audio_widgets.hpp>

using namespace ymir;

namespace app::ui {

SCSPSlotsView::SCSPSlotsView(SharedContext &context)
    : m_context(context)
    , m_scsp(context.saturn.GetSCSP())
    , m_tracer(context.tracers.SCSP) {}

void SCSPSlotsView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();
    const ImVec2 msCharSize = ImGui::CalcTextSize(ICON_MS_KEYBOARD_TAB);
    const auto msCharWidth = msCharSize.x;

    const ImVec2 wfSize{80.0f * m_context.displayScale, msCharSize.y};

    auto &probe = m_scsp.GetProbe();
    const auto &slots = probe.GetSlots();

    ImGui::BeginGroup();

    ImGui::Checkbox("按 SA 着色槽位", &m_colorSlotsBySA);

    const ImVec4 defaultColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

    if (ImGui::BeginTable("slots", 41, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("KYONB", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("SA", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 5);
        ImGui::TableSetupColumn("LSA", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 4);
        ImGui::TableSetupColumn("LEA", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 4);
        ImGui::TableSetupColumn("采样偏移", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 4);
        ImGui::TableSetupColumn("LPCTL", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("位", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("SBCTL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("SSCTL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 4 + paddingWidth);
        ImGui::TableSetupColumn("AR", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("D1R", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("D2R", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("RR", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("DL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("KRS", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 1);
        ImGui::TableSetupColumn("EGHOLD", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("LPSLNK", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("EGBYPASS", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("EG 状态", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 3);
        ImGui::TableSetupColumn("EG 电平", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 3 + paddingWidth);
        ImGui::TableSetupColumn("MDL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 1);
        ImGui::TableSetupColumn("MDXSL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("MDYSL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("STWINH", ImGuiTableColumnFlags_WidthFixed, msCharWidth + paddingWidth);
        ImGui::TableSetupColumn("TL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("SDIR", ImGuiTableColumnFlags_WidthFixed, msCharWidth + paddingWidth);
        ImGui::TableSetupColumn("OCT", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 1);
        ImGui::TableSetupColumn("FNS", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 3);
        ImGui::TableSetupColumn("MSK", ImGuiTableColumnFlags_WidthFixed, msCharWidth + paddingWidth);
        ImGui::TableSetupColumn("LFORE", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("LFOF", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("ALFOS", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("ALFOWS", ImGuiTableColumnFlags_WidthFixed, msCharWidth);
        ImGui::TableSetupColumn("PLFOS", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2);
        ImGui::TableSetupColumn("PLFOWS", ImGuiTableColumnFlags_WidthFixed, msCharWidth + paddingWidth);
        ImGui::TableSetupColumn("IMXL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 1);
        ImGui::TableSetupColumn("ISEL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 1);
        ImGui::TableSetupColumn("DISDL", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 1);
        ImGui::TableSetupColumn("DIPAN", ImGuiTableColumnFlags_WidthFixed, hexCharWidth * 2 + paddingWidth);
        ImGui::TableSetupColumn("输出", ImGuiTableColumnFlags_WidthFixed, wfSize.x);

        // TODO: EFSDL / EFPAN  (probably in DSP view)
        // - slots  0 to 15 = DSP.EFREG[0-15]
        // - slots 16 to 17 = DSP.EXTS[0-1]
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        for (uint32 i = 0; i < 32; ++i) {
            const auto &slot = slots[i];

            ImVec4 color;
            if (m_colorSlotsBySA) {
                color.w = 1.0f;
                // const uint32 value = slot.startAddress;
                // const uint32 value = bit::reverse(slot.startAddress << 13u);
                // const float hue = static_cast<float>(value) / 524287.0f;
                // const uint32 value = bit::extract<0, 8>(slot.startAddress) ^ bit::extract<9, 18>(slot.startAddress);
                const uint32 value =
                    bit::reverse(bit::extract<0, 8>(slot.startAddress) ^ bit::extract<9, 18>(slot.startAddress)) >>
                    (32 - 10);
                const float hue = static_cast<float>(value) / static_cast<float>(0x3FF);
                ImGui::ColorConvertHSVtoRGB(hue, 0.63f, 1.00f, color.x, color.y, color.z);
            } else {
                color = defaultColor;
            }

            const bool disabled = (slot.egState == scsp::Slot::EGState::Release && slot.GetEGLevel() >= 0x3C0) ||
                                  (!slot.active && slot.soundSource == scsp::Slot::SoundSource::SoundRAM);

            if (disabled) {
                ImGui::BeginDisabled();
            }

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                // Index
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02d", i);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // KYONB
                if (slot.keyOnBit) {
                    ImGui::TextColored(color, "%s", ICON_MS_PLAY_ARROW);
                }
            }
            if (ImGui::TableNextColumn()) {
                // SA
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%05X", slot.startAddress);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // LSA
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%04X", slot.loopStartAddress);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // LEA
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%04X", slot.loopEndAddress);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // Sample offset
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%04X", slot.currSample & 0xFFFF);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // LPCTL
                using enum scsp::Slot::LoopControl;
                switch (slot.loopControl) {
                case Off:
                    ImGui::TextColored(color, "%s", ICON_MS_KEYBOARD_TAB);
                    ImGui::SetItemTooltip("无循环");
                    break;
                case Normal:
                    ImGui::TextColored(color, "%s", ICON_MS_ARROW_RIGHT_ALT);
                    ImGui::SetItemTooltip("正向");
                    break;
                case Reverse:
                    ImGui::TextColored(color, "%s", ICON_MS_ARROW_LEFT_ALT);
                    ImGui::SetItemTooltip("反向");
                    break;
                case Alternate:
                    ImGui::TextColored(color, "%s", ICON_MS_ARROW_RANGE);
                    ImGui::SetItemTooltip("交替");
                    break;
                }
            }
            if (ImGui::TableNextColumn()) {
                // PCM8B
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%s", slot.pcm8Bit ? " 8" : "16");
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // SBCTL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.sampleXOR >> 8u);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // SSCTL
                using enum scsp::Slot::SoundSource;
                const char *soundSourceText = "????";
                const char *soundSourceHint = "未知";
                switch (slot.soundSource) {
                case SoundRAM:
                    soundSourceText = "SRAM";
                    soundSourceHint = "声音 RAM";
                    break;
                case Noise:
                    soundSourceText = "LFSR";
                    soundSourceHint = "噪声";
                    break;
                case Silence:
                    soundSourceText = "ZERO";
                    soundSourceHint = "静音";
                    break;
                case Unknown:
                    soundSourceText = "????";
                    soundSourceHint = "未知";
                    break;
                }

                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%s", soundSourceText);
                ImGui::PopFont();
                ImGui::SetItemTooltip("%s", soundSourceHint);
            }

            if (ImGui::TableNextColumn()) {
                // AR
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.attackRate);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // D1R
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.decay1Rate);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // D2R
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.decay2Rate);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // RR
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.releaseRate);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // DL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.decayLevel);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // KRS
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.keyRateScaling);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // EGHOLD
                if (slot.egHold) {
                    ImGui::TextColored(color, "%s", ICON_MS_MAXIMIZE);
                    ImGui::SetItemTooltip("已启用\n"
                                          "EG 电平在起音阶段设置为最大值。");
                } else {
                    ImGui::TextColored(color, "%s", ICON_MS_PEN_SIZE_2);
                    ImGui::SetItemTooltip("已禁用\n"
                                          "EG 电平在起音阶段遵循起音速率。");
                }
            }
            if (ImGui::TableNextColumn()) {
                // LPSLNK
                if (slot.loopStartLink) {
                    ImGui::TextColored(color, "%s", ICON_MS_LINK);
                    ImGui::SetItemTooltip("已启用\n"
                                          "EG 等待直到循环开始才从起音切换到衰减 1 阶段。");
                } else {
                    ImGui::Dummy(msCharSize);
                    ImGui::SetItemTooltip(
                        "已禁用\n"
                        "EG 电平一达到最大值就切换到衰减 1 阶段。");
                }
            }
            if (ImGui::TableNextColumn()) {
                // EGBYPASS
                if (slot.egBypass) {
                    ImGui::TextColored(color, "%s", ICON_MS_STEP_OVER);
                    ImGui::SetItemTooltip("EG 电平被旁路。");
                } else {
                    ImGui::Dummy(msCharSize);
                    ImGui::SetItemTooltip("使用 EG 电平。");
                }
            }
            if (ImGui::TableNextColumn()) {
                // EG state
                using enum scsp::Slot::EGState;

                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                switch (slot.egState) {
                case Attack: ImGui::TextColored(color, "%s", "ATK"); break;
                case Decay1: ImGui::TextColored(color, "%s", "DC1"); break;
                case Decay2: ImGui::TextColored(color, "%s", "DC2"); break;
                case Release: ImGui::TextColored(color, "%s", "REL"); break;
                }
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // EG level
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%03X", slot.GetEGLevel());
                ImGui::PopFont();
            }

            if (ImGui::TableNextColumn()) {
                // MDL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.modLevel);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // MDXSL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.modXSelect);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // MDYSL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.modYSelect);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // STWINH
                if (slot.egBypass) {
                    ImGui::TextColored(color, "%s", ICON_MS_EDIT_OFF);
                    ImGui::SetItemTooltip("槽位输出将不会写入声音栈。");
                } else {
                    ImGui::Dummy(msCharSize);
                    ImGui::SetItemTooltip("槽位输出进入声音栈。");
                }
            }

            if (ImGui::TableNextColumn()) {
                // TL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.totalLevel);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // SDIR
                if (slot.soundDirect) {
                    ImGui::TextColored(color, "%s", ICON_MS_TRENDING_FLAT);
                    ImGui::SetItemTooltip("槽位电平旁路 EG、TL 和 ALFO。");
                } else {
                    ImGui::TextColored(color, "%s", ICON_MS_PLANNER_REVIEW);
                    ImGui::SetItemTooltip("槽位电平包含 EG、TL 和 ALFO。");
                }
            }

            if (ImGui::TableNextColumn()) {
                // OCT
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.octave);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // FNS
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%03X", slot.freqNumSwitch);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // MSK
                if (slot.maskMode) {
                    ImGui::TextColored(color, "%s", ICON_MS_TEXTURE);
                    ImGui::SetItemTooltip("对槽位采样地址使用短波遮罩。");
                } else {
                    ImGui::Dummy(msCharSize);
                    ImGui::SetItemTooltip("未遮罩采样地址。");
                }
            }

            auto drawLFOWaveform = [&](scsp::Slot::Waveform waveform, uint8 sens, bool bipolar) {
                const auto pos = ImGui::GetCursorScreenPos();
                const float padding = 3.0f * m_context.displayScale;
                const ImVec2 wfSize(msCharSize.x - padding * 2.0f, msCharSize.y - padding * 2.0f);
                const ImVec2 basePos(pos.x + padding, pos.y + padding);
                const ImVec2 centerPos(pos.x + msCharSize.x * 0.5f, pos.y + msCharSize.y * 0.5f);
                const ImVec2 endPos(pos.x + msCharSize.x - padding, pos.y + msCharSize.y - padding);

                const float thickness = 1.5f * m_context.displayScale;
                ImVec4 waveColor = color;
                waveColor.w = disabled ? ImGui::GetStyle().DisabledAlpha : 1.0f;
                const ImU32 colorValue = ImGui::ColorConvertFloat4ToU32(waveColor);

                using enum scsp::Slot::Waveform;

                ImGui::Dummy(msCharSize);
                switch (waveform) {
                case Saw: ImGui::SetItemTooltip("锯齿波"); break;
                case Square: ImGui::SetItemTooltip("方波"); break;
                case Triangle: ImGui::SetItemTooltip("三角波"); break;
                case Noise: ImGui::SetItemTooltip("噪声"); break;
                }

                auto *drawList = ImGui::GetWindowDrawList();

                if (sens > 0) {
                    switch (waveform) {
                    case Saw: //
                    {
                        if (bipolar) {
                            const ImVec2 points[] = {
                                {basePos.x, centerPos.y},
                                {centerPos.x, basePos.y},
                                {centerPos.x, endPos.y},
                                {endPos.x, centerPos.y},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        } else {
                            const ImVec2 points[] = {
                                {basePos.x, endPos.y},
                                {endPos.x, basePos.y},
                                {endPos.x, endPos.y},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        }
                        break;
                    }
                    case Square: //
                    {
                        if (bipolar) {
                            const ImVec2 points[] = {
                                {basePos.x + 0.5f, centerPos.y + 0.5f}, {basePos.x + 0.5f, basePos.y + 0.5f},
                                {centerPos.x + 0.5f, basePos.y + 0.5f}, {centerPos.x + 0.5f, endPos.y + 0.5f},
                                {endPos.x + 0.5f, endPos.y + 0.5f},     {endPos.x + 0.5f, centerPos.y + 0.5f},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        } else {
                            const ImVec2 points[] = {
                                {basePos.x + 0.5f, endPos.y + 0.5f},    {basePos.x + 0.5f, basePos.y + 0.5f},
                                {centerPos.x + 0.5f, basePos.y + 0.5f}, {centerPos.x + 0.5f, endPos.y + 0.5f},
                                {endPos.x + 0.5f, endPos.y + 0.5f},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        }
                        break;
                    }
                    case Triangle: //
                    {
                        if (bipolar) {
                            const ImVec2 points[] = {
                                {basePos.x, centerPos.y},
                                {basePos.x + wfSize.x * 0.25f, basePos.y},
                                {basePos.x + wfSize.x * 0.75f, endPos.y},
                                {endPos.x, centerPos.y},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        } else {
                            const ImVec2 points[] = {
                                {basePos.x, endPos.y},
                                {centerPos.x, basePos.y},
                                {endPos.x, endPos.y},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        }
                        break;
                    }
                    case Noise: //
                    {
                        if (bipolar) {
                            const ImVec2 points[] = {
                                {basePos.x + wfSize.x * 0.0f, centerPos.y},
                                {basePos.x + wfSize.x * 0.0f, basePos.y + wfSize.y * 0.135f},
                                {basePos.x + wfSize.x * 0.2f, basePos.y + wfSize.y * 0.135f},
                                {basePos.x + wfSize.x * 0.2f, basePos.y + wfSize.y * 0.968f},
                                {basePos.x + wfSize.x * 0.4f, basePos.y + wfSize.y * 0.968f},
                                {basePos.x + wfSize.x * 0.4f, basePos.y + wfSize.y * 0.437f},
                                {basePos.x + wfSize.x * 0.6f, basePos.y + wfSize.y * 0.437f},
                                {basePos.x + wfSize.x * 0.6f, basePos.y + wfSize.y * 0.016f},
                                {basePos.x + wfSize.x * 0.8f, basePos.y + wfSize.y * 0.016f},
                                {basePos.x + wfSize.x * 0.8f, basePos.y + wfSize.y * 0.811f},
                                {basePos.x + wfSize.x * 1.0f, basePos.y + wfSize.y * 0.811f},
                                {basePos.x + wfSize.x * 1.0f, centerPos.y},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        } else {
                            const ImVec2 points[] = {
                                {basePos.x + wfSize.x * 0.0f, endPos.y},
                                {basePos.x + wfSize.x * 0.0f, basePos.y + wfSize.y * 0.135f},
                                {basePos.x + wfSize.x * 0.2f, basePos.y + wfSize.y * 0.135f},
                                {basePos.x + wfSize.x * 0.2f, basePos.y + wfSize.y * 0.968f},
                                {basePos.x + wfSize.x * 0.4f, basePos.y + wfSize.y * 0.968f},
                                {basePos.x + wfSize.x * 0.4f, basePos.y + wfSize.y * 0.437f},
                                {basePos.x + wfSize.x * 0.6f, basePos.y + wfSize.y * 0.437f},
                                {basePos.x + wfSize.x * 0.6f, basePos.y + wfSize.y * 0.016f},
                                {basePos.x + wfSize.x * 0.8f, basePos.y + wfSize.y * 0.016f},
                                {basePos.x + wfSize.x * 0.8f, basePos.y + wfSize.y * 0.811f},
                                {basePos.x + wfSize.x * 1.0f, basePos.y + wfSize.y * 0.811f},
                                {basePos.x + wfSize.x * 1.0f, endPos.y},
                            };
                            drawList->AddPolyline(points, std::size(points), colorValue, ImDrawFlags_RoundCornersAll,
                                                  thickness);
                        }
                        break;
                    }
                    }
                }
            };

            if (ImGui::TableNextColumn()) {
                // LFORE
                if (slot.lfoReset) {
                    ImGui::TextColored(color, "%s", ICON_MS_REPLAY);
                    ImGui::SetItemTooltip("LFO 将被重置。");
                } else {
                    ImGui::Dummy(msCharSize);
                    ImGui::SetItemTooltip("LFO 将正常递增。");
                }
            }
            if (ImGui::TableNextColumn()) {
                // LFOF
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.lfofRaw);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // ALFOS
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.ampLFOSens);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // ALFOWS
                drawLFOWaveform(slot.ampLFOWaveform, slot.ampLFOSens, false);
            }
            if (ImGui::TableNextColumn()) {
                // PLFOS
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.pitchLFOSens);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // PLFOWS
                drawLFOWaveform(slot.pitchLFOWaveform, slot.pitchLFOSens, true);
            }

            if (ImGui::TableNextColumn()) {
                // IMXL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.inputMixingLevel);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // ISEL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.inputSelect);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // DISDL
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%X", slot.directSendLevel);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // DIPAN
                ImGui::PushFont(m_context.fonts.monospace.regular, m_context.fontSizes.medium);
                ImGui::TextColored(color, "%02X", slot.directPan);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                // Output
                if (m_context.saturn.IsDebugTracingEnabled()) {
                    auto &output = m_tracer.slotOutputs[i];
                    const uint32 len = output.Count();
                    const uint32 max = std::min(len, 512u);
                    const uint32 ofs = len > 512 ? len - 512 : 0;
                    std::array<float, 512> waveform{};
                    for (uint32 j = 0; j < max; ++j) {
                        waveform[j] = output.Read(j + ofs) / 32768.0f;
                    }
                    widgets::Oscilloscope(m_context, std::span{waveform}.first(max), wfSize);
                }
            }

            if (disabled) {
                ImGui::EndDisabled();
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
