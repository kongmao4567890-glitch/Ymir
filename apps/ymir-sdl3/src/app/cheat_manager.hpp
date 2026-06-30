#pragma once

#include <ymir/core/types.hpp>

#include <mutex>
#include <string>
#include <vector>
#include <cstdint>

namespace app {

struct SharedContext;

/// 金手指代码类型
enum class CheatType : uint8 {
    WriteWord,   // 写入16位值到地址 (Action Replay 格式)
    WriteByte,   // 写入8位值到地址
    WriteLong,   // 写入32位值到地址
    Enable,      // 条件启用 (如果地址值等于指定值则写入)
};

/// 单条金手指代码
struct CheatCode {
    CheatType type = CheatType::WriteWord;
    uint32 address = 0;     // 目标地址 (完整 Saturn 地址，如 0x06000000)
    uint32 value = 0;       // 要写入的值
    uint32 compareValue = 0; // Enable 类型的比较值
};

/// 一组金手指（一个作弊码可包含多条写入）
struct CheatEntry {
    std::string name;       // 作弊码名称/描述
    std::string description;// 可选描述
    std::vector<CheatCode> codes; // 该条目包含的代码
    bool enabled = false;   // 是否启用
    bool builtIn = false;   // 是否为内置预设
};

/// 金手指管理器 — 管理所有金手指代码并在每帧应用
class CheatManager {
public:
    CheatManager() = default;

    /// 添加一条金手指
    void AddCheat(const CheatEntry &entry);

    /// 移除指定索引的金手指
    void RemoveCheat(size_t index);

    /// 清空所有金手指
    void ClearAll();

    /// 启用/禁用指定索引的金手指
    void SetEnabled(size_t index, bool enabled);

    /// 获取所有金手指（只读）
    const std::vector<CheatEntry> &GetCheats() const { return m_cheats; }

    /// 获取所有金手指（可写，需加锁）
    std::vector<CheatEntry> &GetCheatsMut() { return m_cheats; }

    /// 获取互斥锁
    std::mutex &GetMutex() { return m_mutex; }

    /// 在模拟线程中每帧调用，应用所有启用的金手指
    void ApplyCheats(SharedContext &ctx);

    /// 解析 Action Replay 格式的金手指代码
    /// 格式: XXXXXXXX YYYY (16进制地址 + 16进制值)
    /// 支持多行，每行一条
    static std::vector<CheatCode> ParseActionReplay(const std::string &text);

    /// 解析单行 Action Replay 代码
    static bool ParseLine(const std::string &line, uint32 &address, uint16 &value);

private:
    std::vector<CheatEntry> m_cheats;
    std::mutex m_mutex;
};

} // namespace app
