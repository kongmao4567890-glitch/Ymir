#include "vdp1_registers_view.hpp"

#include <ymir/hw/vdp/vdp.hpp>

#include <app/events/emu_debug_event_factory.hpp>

#include <imgui.h>

using namespace ymir;

namespace app::ui {

VDP1RegistersView::VDP1RegistersView(SharedContext &context, vdp::VDP &vdp)
    : m_context(context)
    , m_vdp(vdp) {}

void VDP1RegistersView::Display() {
    auto &probe = m_vdp.GetProbe();
    auto reso = probe.GetResolution();
    auto interlace = probe.GetInterlaceMode();
    auto &regs1 = probe.GetVDP1Regs();
    auto &regs2 = probe.GetVDP2Regs();

    static constexpr const char *kInterlaceNames[]{"逐行", "(无效)", "单密度隔行",
                                                   "双密度隔行"};

    auto checkbox = [](const char *name, bool value) { ImGui::Checkbox(name, &value); };

    checkbox(fmt::format("[TVMR.TVM:0] 像素数据: {} 位", regs1.pixel8Bits ? 8u : 16u).c_str(), regs1.pixel8Bits);
    ImGui::Text("VDP2 精灵数据读取大小: %u 位", (regs2.spriteParams.type >= 8 ? 8u : 16u));
    checkbox("[TVMR.TVM:1] 旋转模式", regs1.fbRotEnable);
    checkbox("[TVMR.TVM:2] HDTV 模式", regs1.hdtvEnable);
    checkbox("[FBCR.DIE] 双隔行启用", regs1.dblInterlaceEnable);
    checkbox("[FBCR.DIL] 双隔行绘制偶/奇行", regs1.dblInterlaceDrawLine);
    ImGui::Text("帧缓冲大小: %ux%u", regs1.fbSizeH, regs1.fbSizeV);
    ImGui::Text("VDP2 分辨率: %ux%u %s", reso.width, reso.height, kInterlaceNames[static_cast<uint8>(interlace)]);

    ImGui::Separator();

    checkbox("[TVMR.VBE] VBlank 擦除", regs1.vblankErase);
    checkbox("[FBCR.FCT] 帧缓冲交换触发", regs1.fbSwapTrigger);
    checkbox("[FBCR.FCM] 帧缓冲交换模式", regs1.fbSwapMode);
    ImGui::Indent();
    {
        checkbox("FBCR 已更改", regs1.fbParamsChanged);
    }
    ImGui::Unindent();
    ImGui::Text("[FBCR.PTM] 绘制触发模式: %u", regs1.plotTrigger);
    ImGui::Text("[EWDR] 擦除写入值: 0x%04X", regs1.eraseWriteValue);
    ImGui::Text("[EWLR/EWRR] 擦除窗口: %ux%u - %ux%u", regs1.eraseX1, regs1.eraseY1, regs1.eraseX3,
                regs1.eraseY3);
    ImGui::Indent();
    {
        ImGui::Text("锁存的擦除写入值: 0x%04X", probe.GetLatchedEraseWriteValue());
        ImGui::Text("锁存的擦除窗口: %ux%u - %ux%u", probe.GetLatchedEraseX1(), probe.GetLatchedEraseY1(),
                    probe.GetLatchedEraseX3(), probe.GetLatchedEraseY3());
    }
    ImGui::Unindent();
    checkbox("[FBCR.EOS] 高速缩放偶/奇坐标选择", regs1.evenOddCoordSelect);

    ImGui::Separator();

    checkbox("[EDSR.CEF] 当前帧已结束", regs1.currFrameEnded);
    checkbox("[EDSR.BEF] 上一帧已结束", regs1.prevFrameEnded);
    ImGui::Text("[COPR] 当前帧命令地址: 0x%05X", regs1.currCommandAddress);
    ImGui::Text("[LOPR] 上一帧命令地址: 0x%05X", regs1.prevCommandAddress);
    ImGui::Indent();
    {
        ImGui::Text("返回地址: 0x%05X", regs1.returnAddress);
    }
    ImGui::Unindent();
}

} // namespace app::ui
