#pragma once

#include <app/input/input_action.hpp>

// Note to developers: yes, this could be a single macro, but if you do that your IDE won't autocomplete the functions
#define DEF_ACTION(name) inline constexpr auto name = input::Action
#define ACTION_ID __LINE__

namespace app::actions {

namespace general {

    DEF_ACTION(OpenSettings)::Trigger(ACTION_ID, "常规", "打开设置");
    DEF_ACTION(ToggleWindowedVideoOutput)::Trigger(ACTION_ID, "常规", "切换窗口化视频输出");
    DEF_ACTION(ToggleFullScreen)::Trigger(ACTION_ID, "常规", "切换全屏");
    DEF_ACTION(ShowMessageHistory)::Trigger(ACTION_ID, "常规", "显示消息历史");
    DEF_ACTION(TakeScreenshot)::Trigger(ACTION_ID, "常规", "截图");
    DEF_ACTION(ExitApp)::ComboTrigger(ACTION_ID, "常规", "退出程序");

} // namespace general

namespace view {

    DEF_ACTION(ToggleFrameRateOSD)::Trigger(ACTION_ID, "视图", "切换帧率显示");
    DEF_ACTION(NextFrameRateOSDPos)::Trigger(ACTION_ID, "视图", "下一个帧率显示位置");
    DEF_ACTION(PrevFrameRateOSDPos)::Trigger(ACTION_ID, "视图", "上一个帧率显示位置");

    DEF_ACTION(RotateScreenCW)::Trigger(ACTION_ID, "视图", "顺时针旋转屏幕");
    DEF_ACTION(RotateScreenCCW)::Trigger(ACTION_ID, "视图", "逆时针旋转屏幕");

} // namespace view

namespace audio {

    DEF_ACTION(ToggleMute)::Trigger(ACTION_ID, "音频", "切换静音");
    DEF_ACTION(IncreaseVolume)::RepeatableTrigger(ACTION_ID, "音频", "音量增加 10%");
    DEF_ACTION(DecreaseVolume)::RepeatableTrigger(ACTION_ID, "音频", "音量减少 10%");

} // namespace audio

namespace cd_drive {

    DEF_ACTION(LoadDisc)::Trigger(ACTION_ID, "CD 驱动器", "加载光盘");
    DEF_ACTION(EjectDisc)::Trigger(ACTION_ID, "CD 驱动器", "弹出光盘");
    DEF_ACTION(OpenCloseTray)::Trigger(ACTION_ID, "CD 驱动器", "打开/关闭光驱");

} // namespace cd_drive

namespace save_states {

    DEF_ACTION(QuickLoadState)::Trigger(ACTION_ID, "存档", "快速读取存档");
    DEF_ACTION(QuickSaveState)::Trigger(ACTION_ID, "存档", "快速保存存档");

    DEF_ACTION(SelectState1)::Trigger(ACTION_ID, "存档", "选择存档槽 1");
    DEF_ACTION(SelectState2)::Trigger(ACTION_ID, "存档", "选择存档槽 2");
    DEF_ACTION(SelectState3)::Trigger(ACTION_ID, "存档", "选择存档槽 3");
    DEF_ACTION(SelectState4)::Trigger(ACTION_ID, "存档", "选择存档槽 4");
    DEF_ACTION(SelectState5)::Trigger(ACTION_ID, "存档", "选择存档槽 5");
    DEF_ACTION(SelectState6)::Trigger(ACTION_ID, "存档", "选择存档槽 6");
    DEF_ACTION(SelectState7)::Trigger(ACTION_ID, "存档", "选择存档槽 7");
    DEF_ACTION(SelectState8)::Trigger(ACTION_ID, "存档", "选择存档槽 8");
    DEF_ACTION(SelectState9)::Trigger(ACTION_ID, "存档", "选择存档槽 9");
    DEF_ACTION(SelectState10)::Trigger(ACTION_ID, "存档", "选择存档槽 10");

    DEF_ACTION(LoadState1)::Trigger(ACTION_ID, "存档", "读取存档 1");
    DEF_ACTION(LoadState2)::Trigger(ACTION_ID, "存档", "读取存档 2");
    DEF_ACTION(LoadState3)::Trigger(ACTION_ID, "存档", "读取存档 3");
    DEF_ACTION(LoadState4)::Trigger(ACTION_ID, "存档", "读取存档 4");
    DEF_ACTION(LoadState5)::Trigger(ACTION_ID, "存档", "读取存档 5");
    DEF_ACTION(LoadState6)::Trigger(ACTION_ID, "存档", "读取存档 6");
    DEF_ACTION(LoadState7)::Trigger(ACTION_ID, "存档", "读取存档 7");
    DEF_ACTION(LoadState8)::Trigger(ACTION_ID, "存档", "读取存档 8");
    DEF_ACTION(LoadState9)::Trigger(ACTION_ID, "存档", "读取存档 9");
    DEF_ACTION(LoadState10)::Trigger(ACTION_ID, "存档", "读取存档 10");

    DEF_ACTION(SaveState1)::Trigger(ACTION_ID, "存档", "保存存档 1");
    DEF_ACTION(SaveState2)::Trigger(ACTION_ID, "存档", "保存存档 2");
    DEF_ACTION(SaveState3)::Trigger(ACTION_ID, "存档", "保存存档 3");
    DEF_ACTION(SaveState4)::Trigger(ACTION_ID, "存档", "保存存档 4");
    DEF_ACTION(SaveState5)::Trigger(ACTION_ID, "存档", "保存存档 5");
    DEF_ACTION(SaveState6)::Trigger(ACTION_ID, "存档", "保存存档 6");
    DEF_ACTION(SaveState7)::Trigger(ACTION_ID, "存档", "保存存档 7");
    DEF_ACTION(SaveState8)::Trigger(ACTION_ID, "存档", "保存存档 8");
    DEF_ACTION(SaveState9)::Trigger(ACTION_ID, "存档", "保存存档 9");
    DEF_ACTION(SaveState10)::Trigger(ACTION_ID, "存档", "保存存档 10");

    inline const input::Action &GetSelectStateAction(uint32 index) {
        switch (index) {
        default: [[fallthrough]];
        case 0: return SelectState1;
        case 1: return SelectState2;
        case 2: return SelectState3;
        case 3: return SelectState4;
        case 4: return SelectState5;
        case 5: return SelectState6;
        case 6: return SelectState7;
        case 7: return SelectState8;
        case 8: return SelectState9;
        case 9: return SelectState10;
        }
    }

    inline const input::Action &GetLoadStateAction(uint32 index) {
        switch (index) {
        default: [[fallthrough]];
        case 0: return LoadState1;
        case 1: return LoadState2;
        case 2: return LoadState3;
        case 3: return LoadState4;
        case 4: return LoadState5;
        case 5: return LoadState6;
        case 6: return LoadState7;
        case 7: return LoadState8;
        case 8: return LoadState9;
        case 9: return LoadState10;
        }
    }

    inline const input::Action &GetSaveStateAction(uint32 index) {
        switch (index) {
        default: [[fallthrough]];
        case 0: return SaveState1;
        case 1: return SaveState2;
        case 2: return SaveState3;
        case 3: return SaveState4;
        case 4: return SaveState5;
        case 5: return SaveState6;
        case 6: return SaveState7;
        case 7: return SaveState8;
        case 8: return SaveState9;
        case 9: return SaveState10;
        }
    }

    DEF_ACTION(UndoLoadState)::Trigger(ACTION_ID, "存档", "撤销读取存档");
    DEF_ACTION(UndoSaveState)::Trigger(ACTION_ID, "存档", "撤销保存存档");

} // namespace save_states

namespace sys {

    DEF_ACTION(HardReset)::Trigger(ACTION_ID, "系统", "硬重启");
    DEF_ACTION(SoftReset)::Trigger(ACTION_ID, "系统", "软重启");
    DEF_ACTION(ResetButton)::Button(ACTION_ID, "系统", "复位按钮");

} // namespace sys

namespace emu {

    DEF_ACTION(TurboSpeed)::Button(ACTION_ID, "模拟", "加速");
    DEF_ACTION(TurboSpeedHold)::Trigger(ACTION_ID, "模拟", "加速（按住）");
    DEF_ACTION(ToggleAlternateSpeed)::Trigger(ACTION_ID, "模拟", "切换备用速度");
    DEF_ACTION(IncreaseSpeed)::RepeatableTrigger(ACTION_ID, "模拟", "速度增加 5%");
    DEF_ACTION(DecreaseSpeed)::RepeatableTrigger(ACTION_ID, "模拟", "速度减少 5%");
    DEF_ACTION(IncreaseSpeedLarge)::RepeatableTrigger(ACTION_ID, "模拟", "速度增加 25%");
    DEF_ACTION(DecreaseSpeedLarge)::RepeatableTrigger(ACTION_ID, "模拟", "速度减少 25%");
    DEF_ACTION(ResetSpeed)::Trigger(ACTION_ID, "模拟", "重置速度");

    DEF_ACTION(PauseResume)::Trigger(ACTION_ID, "模拟", "暂停/继续");
    DEF_ACTION(ForwardFrameStep)::RepeatableTrigger(ACTION_ID, "模拟", "向前帧步进");
    DEF_ACTION(ReverseFrameStep)::RepeatableTrigger(ACTION_ID, "模拟", "向后帧步进");
    DEF_ACTION(Rewind)::Button(ACTION_ID, "模拟", "回退");

    DEF_ACTION(ToggleRewindBuffer)::Trigger(ACTION_ID, "模拟", "切换回退缓冲");

} // namespace emu

namespace dbg {

    DEF_ACTION(ToggleDebugTrace)::Trigger(ACTION_ID, "调试器", "切换跟踪");
    DEF_ACTION(DumpMemory)::Trigger(ACTION_ID, "调试器", "转储所有内存");

} // namespace dbg

namespace control_pad {

    DEF_ACTION(A)::Button(ACTION_ID, "土星手柄", "A");
    DEF_ACTION(B)::Button(ACTION_ID, "土星手柄", "B");
    DEF_ACTION(C)::Button(ACTION_ID, "土星手柄", "C");
    DEF_ACTION(X)::Button(ACTION_ID, "土星手柄", "X");
    DEF_ACTION(Y)::Button(ACTION_ID, "土星手柄", "Y");
    DEF_ACTION(Z)::Button(ACTION_ID, "土星手柄", "Z");
    DEF_ACTION(L)::Button(ACTION_ID, "土星手柄", "L");
    DEF_ACTION(R)::Button(ACTION_ID, "土星手柄", "R");
    DEF_ACTION(Start)::Button(ACTION_ID, "土星手柄", "Start");
    DEF_ACTION(Up)::Button(ACTION_ID, "土星手柄", "上");
    DEF_ACTION(Down)::Button(ACTION_ID, "土星手柄", "下");
    DEF_ACTION(Left)::Button(ACTION_ID, "土星手柄", "左");
    DEF_ACTION(Right)::Button(ACTION_ID, "土星手柄", "右");
    DEF_ACTION(DPad)::AbsoluteBipolarAxis2D(ACTION_ID, "土星手柄", "方向键轴");

} // namespace control_pad

namespace analog_pad {

    DEF_ACTION(A)::Button(ACTION_ID, "土星 3D 手柄", "A");
    DEF_ACTION(B)::Button(ACTION_ID, "土星 3D 手柄", "B");
    DEF_ACTION(C)::Button(ACTION_ID, "土星 3D 手柄", "C");
    DEF_ACTION(X)::Button(ACTION_ID, "土星 3D 手柄", "X");
    DEF_ACTION(Y)::Button(ACTION_ID, "土星 3D 手柄", "Y");
    DEF_ACTION(Z)::Button(ACTION_ID, "土星 3D 手柄", "Z");
    DEF_ACTION(L)::Button(ACTION_ID, "土星 3D 手柄", "L");
    DEF_ACTION(R)::Button(ACTION_ID, "土星 3D 手柄", "R");
    DEF_ACTION(Start)::Button(ACTION_ID, "土星 3D 手柄", "Start");
    DEF_ACTION(Up)::Button(ACTION_ID, "土星 3D 手柄", "上");
    DEF_ACTION(Down)::Button(ACTION_ID, "土星 3D 手柄", "下");
    DEF_ACTION(Left)::Button(ACTION_ID, "土星 3D 手柄", "左");
    DEF_ACTION(Right)::Button(ACTION_ID, "土星 3D 手柄", "右");
    DEF_ACTION(DPad)::AbsoluteBipolarAxis2D(ACTION_ID, "土星 3D 手柄", "方向键轴");
    DEF_ACTION(AnalogStick)::AbsoluteBipolarAxis2D(ACTION_ID, "土星 3D 手柄", "模拟摇杆");
    DEF_ACTION(AnalogL)::AbsoluteMonopolarAxis1D(ACTION_ID, "土星 3D 手柄", "模拟 L");
    DEF_ACTION(AnalogR)::AbsoluteMonopolarAxis1D(ACTION_ID, "土星 3D 手柄", "模拟 R");
    DEF_ACTION(SwitchMode)::Trigger(ACTION_ID, "土星 3D 手柄", "切换模式");

} // namespace analog_pad

namespace arcade_racer {

    DEF_ACTION(A)::Button(ACTION_ID, "街机赛车方向盘", "A");
    DEF_ACTION(B)::Button(ACTION_ID, "街机赛车方向盘", "B");
    DEF_ACTION(C)::Button(ACTION_ID, "街机赛车方向盘", "C");
    DEF_ACTION(X)::Button(ACTION_ID, "街机赛车方向盘", "X");
    DEF_ACTION(Y)::Button(ACTION_ID, "街机赛车方向盘", "Y");
    DEF_ACTION(Z)::Button(ACTION_ID, "街机赛车方向盘", "Z");
    DEF_ACTION(Start)::Button(ACTION_ID, "街机赛车方向盘", "Start");
    DEF_ACTION(GearUp)::Button(ACTION_ID, "街机赛车方向盘", "升档");
    DEF_ACTION(GearDown)::Button(ACTION_ID, "街机赛车方向盘", "降档");
    DEF_ACTION(WheelLeft)::Button(ACTION_ID, "街机赛车方向盘", "方向盘左转");
    DEF_ACTION(WheelRight)::Button(ACTION_ID, "街机赛车方向盘", "方向盘右转");
    DEF_ACTION(AnalogWheel)::AbsoluteBipolarAxis1D(ACTION_ID, "街机赛车方向盘", "方向盘（模拟）");

} // namespace arcade_racer

namespace mission_stick {

    DEF_ACTION(A)::Button(ACTION_ID, "任务摇杆", "A");
    DEF_ACTION(B)::Button(ACTION_ID, "任务摇杆", "B");
    DEF_ACTION(C)::Button(ACTION_ID, "任务摇杆", "C");
    DEF_ACTION(X)::Button(ACTION_ID, "任务摇杆", "X");
    DEF_ACTION(Y)::Button(ACTION_ID, "任务摇杆", "Y");
    DEF_ACTION(Z)::Button(ACTION_ID, "任务摇杆", "Z");
    DEF_ACTION(L)::Button(ACTION_ID, "任务摇杆", "L");
    DEF_ACTION(R)::Button(ACTION_ID, "任务摇杆", "R");
    DEF_ACTION(Start)::Button(ACTION_ID, "任务摇杆", "Start");
    DEF_ACTION(MainUp)::Button(ACTION_ID, "任务摇杆", "主摇杆上");
    DEF_ACTION(MainDown)::Button(ACTION_ID, "任务摇杆", "主摇杆下");
    DEF_ACTION(MainLeft)::Button(ACTION_ID, "任务摇杆", "主摇杆左");
    DEF_ACTION(MainRight)::Button(ACTION_ID, "任务摇杆", "主摇杆右");
    DEF_ACTION(MainStick)::AbsoluteBipolarAxis2D(ACTION_ID, "任务摇杆", "主摇杆");
    DEF_ACTION(MainThrottle)::AbsoluteMonopolarAxis1D(ACTION_ID, "任务摇杆", "主油门");
    DEF_ACTION(MainThrottleUp)::RepeatableTrigger(ACTION_ID, "任务摇杆", "主油门加大");
    DEF_ACTION(MainThrottleDown)::RepeatableTrigger(ACTION_ID, "任务摇杆", "主油门减小");
    DEF_ACTION(MainThrottleMax)::Trigger(ACTION_ID, "任务摇杆", "主油门最大");
    DEF_ACTION(MainThrottleMin)::Trigger(ACTION_ID, "任务摇杆", "主油门最小");
    DEF_ACTION(SubUp)::Button(ACTION_ID, "任务摇杆", "副摇杆上");
    DEF_ACTION(SubDown)::Button(ACTION_ID, "任务摇杆", "副摇杆下");
    DEF_ACTION(SubLeft)::Button(ACTION_ID, "任务摇杆", "副摇杆左");
    DEF_ACTION(SubRight)::Button(ACTION_ID, "任务摇杆", "副摇杆右");
    DEF_ACTION(SubStick)::AbsoluteBipolarAxis2D(ACTION_ID, "任务摇杆", "副摇杆");
    DEF_ACTION(SubThrottle)::AbsoluteMonopolarAxis1D(ACTION_ID, "任务摇杆", "副油门");
    DEF_ACTION(SubThrottleUp)::RepeatableTrigger(ACTION_ID, "任务摇杆", "副油门加大");
    DEF_ACTION(SubThrottleDown)::RepeatableTrigger(ACTION_ID, "任务摇杆", "副油门减小");
    DEF_ACTION(SubThrottleMax)::Trigger(ACTION_ID, "任务摇杆", "副油门最大");
    DEF_ACTION(SubThrottleMin)::Trigger(ACTION_ID, "任务摇杆", "副油门最小");
    DEF_ACTION(SwitchMode)::Trigger(ACTION_ID, "任务摇杆", "切换模式");

} // namespace mission_stick

namespace virtua_gun {

    DEF_ACTION(Start)::Button(ACTION_ID, "光枪", "Start");
    DEF_ACTION(Trigger)::Button(ACTION_ID, "光枪", "扳机");
    DEF_ACTION(Reload)::Button(ACTION_ID, "光枪", "装填");
    DEF_ACTION(Up)::Button(ACTION_ID, "光枪", "上移");
    DEF_ACTION(Down)::Button(ACTION_ID, "光枪", "下移");
    DEF_ACTION(Left)::Button(ACTION_ID, "光枪", "左移");
    DEF_ACTION(Right)::Button(ACTION_ID, "光枪", "右移");
    DEF_ACTION(Move)::AbsoluteBipolarAxis2D(ACTION_ID, "光枪", "移动轴");
    DEF_ACTION(Recenter)::Trigger(ACTION_ID, "光枪", "回中");
    DEF_ACTION(SpeedBoost)::Button(ACTION_ID, "光枪", "加速");
    DEF_ACTION(SpeedToggle)::Trigger(ACTION_ID, "光枪", "切换速度");

    // Hidden actions
    DEF_ACTION(MouseRelMove)::RelativeBipolarAxis2D(ACTION_ID, "光枪", "移动（相对鼠标）");
    DEF_ACTION(MouseAbsMove)::AbsoluteBipolarAxis2D(ACTION_ID, "光枪", "移动（绝对鼠标）");

} // namespace virtua_gun

namespace shuttle_mouse {

    DEF_ACTION(Start)::Button(ACTION_ID, "穿梭鼠标", "Start");
    DEF_ACTION(Left)::Button(ACTION_ID, "穿梭鼠标", "左键");
    DEF_ACTION(Middle)::Button(ACTION_ID, "穿梭鼠标", "中键");
    DEF_ACTION(Right)::Button(ACTION_ID, "穿梭鼠标", "右键");
    DEF_ACTION(MoveUp)::Button(ACTION_ID, "穿梭鼠标", "上移");
    DEF_ACTION(MoveDown)::Button(ACTION_ID, "穿梭鼠标", "下移");
    DEF_ACTION(MoveLeft)::Button(ACTION_ID, "穿梭鼠标", "左移");
    DEF_ACTION(MoveRight)::Button(ACTION_ID, "穿梭鼠标", "右移");
    DEF_ACTION(Move)::RelativeBipolarAxis2D(ACTION_ID, "穿梭鼠标", "移动轴");
    DEF_ACTION(SpeedBoost)::Button(ACTION_ID, "穿梭鼠标", "加速");
    DEF_ACTION(SpeedToggle)::Trigger(ACTION_ID, "穿梭鼠标", "切换速度");

    // Hidden actions
    DEF_ACTION(MouseRelMove)::RelativeBipolarAxis2D(ACTION_ID, "穿梭鼠标", "移动（相对鼠标）");

} // namespace shuttle_mouse

} // namespace app::actions

#undef DEF_ACTION
#undef ACTION_ID
