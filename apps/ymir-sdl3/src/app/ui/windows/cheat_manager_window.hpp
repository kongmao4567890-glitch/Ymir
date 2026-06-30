#pragma once

#include <app/cheat_manager.hpp>
#include <app/ui/window_base.hpp>

#include <string>

namespace app::ui {

/// 金手指管理窗口
class CheatManagerWindow : public WindowBase {
public:
    CheatManagerWindow(SharedContext &context, CheatManager &cheatManager);

protected:
    void DrawContents() override;

private:
    CheatManager &m_cheatManager;

    // 添加金手指的输入缓冲
    char m_nameBuf[256] = {};
    char m_codeBuf[4096] = {};

    // 导入/导出文件路径缓冲
    char m_filePathBuf[512] = {};

    // 选中的金手指索引
    int m_selectedIndex = -1;
};

} // namespace app::ui
