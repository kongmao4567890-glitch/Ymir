#include "cheat_manager.hpp"
#include "shared_context.hpp"

#include <ymir/sys/bus.hpp>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace app {

void CheatManager::AddCheat(const CheatEntry &entry) {
    std::lock_guard lock(m_mutex);
    m_cheats.push_back(entry);
}

void CheatManager::RemoveCheat(size_t index) {
    std::lock_guard lock(m_mutex);
    if (index < m_cheats.size()) {
        m_cheats.erase(m_cheats.begin() + index);
    }
}

void CheatManager::ClearAll() {
    std::lock_guard lock(m_mutex);
    m_cheats.clear();
}

void CheatManager::SetEnabled(size_t index, bool enabled) {
    std::lock_guard lock(m_mutex);
    if (index < m_cheats.size()) {
        m_cheats[index].enabled = enabled;
    }
}

void CheatManager::ApplyCheats(SharedContext &ctx) {
    std::lock_guard lock(m_mutex);
    if (!ctx.saturn.instance) {
        return;
    }

    auto &bus = ctx.saturn.GetMainBus();

    for (auto &entry : m_cheats) {
        if (!entry.enabled) {
            continue;
        }
        for (auto &code : entry.codes) {
            switch (code.type) {
            case CheatType::WriteByte:
                bus.Poke<uint8>(code.address, (uint8)code.value);
                break;
            case CheatType::WriteWord:
                bus.Poke<uint16>(code.address, (uint16)code.value);
                break;
            case CheatType::WriteLong:
                bus.Poke<uint32>(code.address, code.value);
                break;
            case CheatType::Enable: {
                uint16 current = bus.Peek<uint16>(code.address);
                if (current == (uint16)code.compareValue) {
                    bus.Poke<uint16>(code.address, (uint16)code.value);
                }
                break;
            }
            }
        }
    }
}

bool CheatManager::ParseLine(const std::string &line, uint32 &address, uint16 &value) {
    // 跳过空行和注释
    std::string trimmed;
    for (char c : line) {
        if (std::isalnum((unsigned char)c)) {
            trimmed += c;
        } else if (c == ' ' || c == '\t') {
            if (!trimmed.empty() && trimmed.back() != ' ') {
                trimmed += ' ';
            }
        }
    }
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == '/') {
        return false;
    }

    // 尝试解析 "XXXXXXXX YYYY" 格式
    std::istringstream iss(trimmed);
    std::string addrStr, valStr;
    if (!(iss >> addrStr >> valStr)) {
        return false;
    }

    try {
        address = std::stoul(addrStr, nullptr, 16);
        value = (uint16)std::stoul(valStr, nullptr, 16);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<CheatCode> CheatManager::ParseActionReplay(const std::string &text) {
    std::vector<CheatCode> codes;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        uint32 address = 0;
        uint16 value = 0;
        if (ParseLine(line, address, value)) {
            CheatCode code;
            // Action Replay 标准格式为 16 位写入
            code.type = CheatType::WriteWord;
            code.address = address;
            code.value = value;
            codes.push_back(code);
        }
    }

    return codes;
}

} // namespace app
