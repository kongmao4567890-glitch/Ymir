#include "cartridge_settings_view.hpp"

#include <ymir/hw/cart/cart.hpp>
#include <ymir/hw/cdblock/cdblock.hpp>
#include <ymir/sys/backup_ram.hpp>

#include <ymir/db/game_db.hpp>

#include <app/ui/widgets/cartridge_widgets.hpp>
#include <app/ui/widgets/common_widgets.hpp>

#include <app/events/emu_event_factory.hpp>
#include <app/events/gui_event_factory.hpp>

#include <util/file_loader.hpp>
#include <util/sdl_file_dialog.hpp>

#include <misc/cpp/imgui_stdlib.h>

#include <fmt/std.h>

#include <SDL3/SDL_misc.h>

using namespace ymir;

namespace app::ui {

static const char *GetCartTypeName(Settings::Cartridge::Type type) {
    switch (type) {
    case Settings::Cartridge::Type::None: return "无";
    case Settings::Cartridge::Type::BackupRAM: return "备份 RAM";
    case Settings::Cartridge::Type::DRAM: return "DRAM";
    case Settings::Cartridge::Type::ROM: return "ROM";
    default: return "未知";
    }
}

// ---------------------------------------------------------------------------------------------------------------------

CartridgeSettingsView::CartridgeSettingsView(SharedContext &context)
    : SettingsViewBase(context) {}

void CartridgeSettingsView::Display() {
    auto &settings = GetSettings().cartridge;

    static constexpr Settings::Cartridge::Type kCartTypes[] = {
        Settings::Cartridge::Type::None,
        Settings::Cartridge::Type::BackupRAM,
        Settings::Cartridge::Type::DRAM,
        Settings::Cartridge::Type::ROM,
    };

    ImGui::PushTextWrapPos(ImGui::GetContentRegionMax().x);

    ImGui::TextUnformatted("当前卡带：");
    ImGui::SameLine(0, 0);
    widgets::CartridgeInfo(m_context);
    {
        std::unique_lock lock{m_context.locks.cart};
        auto &cart = m_context.saturn.GetCartridge();
        if (cart.GetType() == cart::CartType::BackupMemory) {
            auto &bupCart = *cart.As<cart::CartType::BackupMemory>();
            ImGui::Text("镜像路径：%s", fmt::format("{}", bupCart.GetBackupMemory().GetPath()).c_str());
        }
    }

    MakeDirty(ImGui::Checkbox("自动切换到推荐的卡带", &settings.autoLoadGameCarts));
    widgets::ExplanationTooltip(
        fmt::format("某些游戏需要特定的卡带才能运行：\n"
                    "- The King of Fighters '95 和 Ultraman 需要各自的 ROM 卡带\n"
                    "- 其他一些游戏需要 DRAM 卡带才能启动\n"
                    "\n"
                    "启用此选项后，当加载有这些需求的游戏时，会自动插入正确的卡带。\n"
                    "\n"
                    "对于 ROM 卡带，请确保将所需文件添加到 {}。",
                    m_context.profile.GetPath(ProfilePath::ROMCartImages))
            .c_str(),
        m_context.displayScale);

    if (ImGui::Button("打开 ROM 卡带镜像目录")) {
        SDL_OpenURL(fmt::format("file:///{}", m_context.profile.GetPath(ProfilePath::ROMCartImages)).c_str());
    }

    ImGui::Separator();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("卡带类型：");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##cart_type", GetCartTypeName(settings.type), ImGuiComboFlags_WidthFitPreview)) {
        for (auto type : kCartTypes) {
            if (MakeDirty(ImGui::Selectable(GetCartTypeName(type), type == settings.type))) {
                settings.type = type;
            }
        }

        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("插入")) {
        m_context.EnqueueEvent(events::emu::InsertCartridgeFromSettings());
    }

    const db::GameInfo *gameInfo = nullptr;
    {
        std::unique_lock lock{m_context.locks.disc};
        const auto &disc = m_context.saturn.GetDisc();
        if (!disc.sessions.empty()) {
            gameInfo = db::GetGameInfo(disc.header.productNumber, m_context.saturn.GetDiscHash());
        }
    }
    if (gameInfo != nullptr) {
        cart::CartType wantedCartType;
        switch (gameInfo->GetCartridge()) {
        case db::Cartridge::DRAM8Mbit: wantedCartType = cart::CartType::DRAM8Mbit; break;
        case db::Cartridge::DRAM32Mbit: wantedCartType = cart::CartType::DRAM32Mbit; break;
        case db::Cartridge::DRAM48Mbit: wantedCartType = cart::CartType::DRAM48Mbit; break;
        case db::Cartridge::ROM_KOF95: wantedCartType = cart::CartType::ROM; break;
        case db::Cartridge::ROM_Ultraman: wantedCartType = cart::CartType::ROM; break;
        case db::Cartridge::BackupRAM: wantedCartType = cart::CartType::BackupMemory; break;
        default: wantedCartType = cart::CartType::None; break;
        }

        cart::CartType currCartType;
        {
            std::unique_lock lock{m_context.locks.cart};
            currCartType = m_context.saturn.GetCartridge().GetType();
        }

        if (wantedCartType != cart::CartType::None && currCartType != wantedCartType) {
            ImGui::AlignTextToFramePadding();

            const auto color = m_context.colors.notice;

            switch (gameInfo->GetCartridge()) {
            case db::Cartridge::DRAM8Mbit:
                ImGui::TextColored(color, "当前加载的游戏需要 8 Mbit DRAM 卡带。");
                break;
            case db::Cartridge::DRAM32Mbit:
                ImGui::TextColored(color, "当前加载的游戏需要 32 Mbit DRAM 卡带。");
                break;
            case db::Cartridge::DRAM48Mbit:
                ImGui::TextColored(color, "当前加载的游戏需要 48 Mbit DRAM 开发卡带。");
                break;
            case db::Cartridge::ROM_KOF95:
                ImGui::TextColored(color, "当前加载的游戏需要 King of Fighters '95 ROM 卡带。");
                break;
            case db::Cartridge::ROM_Ultraman:
                ImGui::TextColored(color, "当前加载的游戏需要 Ultraman ROM 卡带。");
                break;
            case db::Cartridge::BackupRAM:
                ImGui::TextColored(color, "此游戏推荐使用备份 RAM 卡带。");
                break;
            default: break;
            }
            ImGui::SameLine();

            if (MakeDirty(ImGui::Button("插入##recommended_cart"))) {
                m_context.EnqueueEvent(events::gui::LoadRecommendedGameCartridge());
            }

            if (gameInfo->cartReason) {
                ImGui::TextColored(color, "原因：%s", gameInfo->cartReason);
            }
        }
    }

    switch (settings.type) {
    case Settings::Cartridge::Type::None: break;
    case Settings::Cartridge::Type::BackupRAM: DrawBackupRAMSettings(); break;
    case Settings::Cartridge::Type::DRAM: DrawDRAMSettings(); break;
    case Settings::Cartridge::Type::ROM: DrawROMSettings(); break;
    }

    ImGui::PopTextWrapPos();
}

void CartridgeSettingsView::DrawBackupRAMSettings() {
    auto &settings = GetSettings().cartridge.backupRAM;

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
    const float fileSelectorButtonWidth = ImGui::CalcTextSize("...").x + paddingWidth * 2;

    using BUPCap = Settings::Cartridge::BackupRAM::Capacity;

    uint32 currSize = 0;
    std::filesystem::path currPath = "";
    {
        std::unique_lock lock{m_context.locks.cart};
        auto *cart = m_context.saturn.GetCartridge().As<cart::CartType::BackupMemory>();
        if (cart != nullptr) {
            auto &bupMem = cart->GetBackupMemory();
            currSize = bupMem.Size();
            currPath = bupMem.GetPath();
        }
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("容量：");
    widgets::ExplanationTooltip(
        "如果你从下面的文件选择器加载现有镜像，这将自动调整。",
        m_context.displayScale);
    ImGui::SameLine();
    if (ImGui::BeginCombo("##bup_capacity", BupCapacityLongName(settings.capacity), ImGuiComboFlags_WidthFitPreview)) {
        for (auto cap : {BUPCap::_4Mbit, BUPCap::_8Mbit, BUPCap::_16Mbit, BUPCap::_32Mbit}) {
            if (MakeDirty(ImGui::Selectable(BupCapacityLongName(cap), settings.capacity == cap))) {
                settings.capacity = cap;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("镜像路径：");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-(fileSelectorButtonWidth + itemSpacingWidth * 2));
    std::string imagePath = fmt::format("{}", settings.imagePath);
    std::string defaultPath =
        fmt::format("{}", m_context.profile.GetPath(ProfilePath::PersistentState) /
                              fmt::format("bup-ext-{}M.bin", CapacityToSize(settings.capacity) * 8 / 1024 / 1024));
    if (MakeDirty(ImGui::InputTextWithHint("##bup_image_path", defaultPath.c_str(), &imagePath,
                                           ImGuiInputTextFlags_ElideLeft))) {
        settings.imagePath = std::u8string{imagePath.begin(), imagePath.end()};
    }
    ImGui::SameLine();
    if (ImGui::Button("...##bup_image_path")) {
        m_context.EnqueueEvent(events::gui::SaveFile({
            .dialogTitle = "加载备份内存镜像",
            .defaultPath = settings.imagePath.empty()
                               ? m_context.profile.GetPath(ProfilePath::PersistentState) / "bup-ext.bin"
                               : settings.imagePath,
            .filters = {{"备份内存镜像文件 (*.bin, *.sav)", "bin;sav"}, {"所有文件 (*.*)", "*"}},
            .userdata = this,
            .callback = util::WrapSingleSelectionCallback<&CartridgeSettingsView::ProcessLoadBackupImage,
                                                          &util::NoopCancelFileDialogCallback,
                                                          &CartridgeSettingsView::ProcessLoadBackupImageError>,
        }));
    }

    if (ImGui::Button("打开备份内存管理器")) {
        m_context.EnqueueEvent(events::gui::OpenBackupMemoryManager());
    }

    if (currSize != 0 && CapacityToSize(settings.capacity) != currSize && !currPath.empty() &&
        !settings.imagePath.empty() && currPath == settings.imagePath) {
        ImGui::TextUnformatted("警告：更改备份内存镜像的大小将格式化它！");
    }
}

void CartridgeSettingsView::DrawDRAMSettings() {
    auto &settings = GetSettings().cartridge.dram;

    using DRAMCap = Settings::Cartridge::DRAM::Capacity;

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("容量：");
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("48 Mbit (6 MiB)（开发）", settings.capacity == DRAMCap::_48Mbit))) {
        settings.capacity = DRAMCap::_48Mbit;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("32 Mbit (4 MiB)", settings.capacity == DRAMCap::_32Mbit))) {
        settings.capacity = DRAMCap::_32Mbit;
    }
    ImGui::SameLine();
    if (MakeDirty(ImGui::RadioButton("8 Mbit (1 MiB)", settings.capacity == DRAMCap::_8Mbit))) {
        settings.capacity = DRAMCap::_8Mbit;
    }
}

void CartridgeSettingsView::DrawROMSettings() {
    auto &settings = GetSettings().cartridge.rom;

    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingWidth = ImGui::GetStyle().ItemSpacing.x;
    const float fileSelectorButtonWidth = ImGui::CalcTextSize("...").x + paddingWidth * 2;

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("镜像路径：");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-(fileSelectorButtonWidth + itemSpacingWidth * 2));
    std::string imagePath = fmt::format("{}", settings.imagePath);
    if (MakeDirty(ImGui::InputText("##rom_image_path", &imagePath, ImGuiInputTextFlags_ElideLeft))) {
        settings.imagePath = std::u8string{imagePath.begin(), imagePath.end()};
    }
    ImGui::SameLine();
    if (ImGui::Button("...##rom_image_path")) {
        m_context.EnqueueEvent(events::gui::OpenFile({
            .dialogTitle = "加载 ROM 卡带镜像",
            .defaultPath =
                settings.imagePath.empty() ? m_context.profile.GetPath(ProfilePath::ROMCartImages) : settings.imagePath,
            .filters = {{"ROM 卡带镜像文件 (*.bin, *.ic1)", "bin;ic1"}, {"所有文件 (*.*)", "*"}},
            .userdata = this,
            .callback = util::WrapSingleSelectionCallback<&CartridgeSettingsView::ProcessLoadROMImage,
                                                          &util::NoopCancelFileDialogCallback,
                                                          &CartridgeSettingsView::ProcessLoadROMImageError>,
        }));
    }
}

void CartridgeSettingsView::ProcessLoadBackupImage(void *userdata, std::filesystem::path file, int filter) {
    static_cast<CartridgeSettingsView *>(userdata)->LoadBackupImage(file);
}

void CartridgeSettingsView::ProcessLoadBackupImageError(void *userdata, const char *message, int filter) {
    static_cast<CartridgeSettingsView *>(userdata)->ShowLoadBackupImageError(message);
}

void CartridgeSettingsView::LoadBackupImage(std::filesystem::path file) {
    auto &settings = GetSettings().cartridge.backupRAM;

    // TODO: rework this entire process
    // - everything here should be done by the emulator event
    // - it should also reuse the current backup cartridge and ask the bup::BackupMemory to LoadFrom/CreateFrom
    //   - CreateFrom should temporarily disconnect the file in order to modify its size if it's trying to change
    //   the
    //     file it has already loaded

    std::error_code error{};
    bup::BackupMemory bupMem{};
    if (std::filesystem::is_regular_file(file)) {
        // The user selected an existing image. Make sure it's a proper backup image.
        const auto result = bupMem.LoadFrom(file, false, error);
        switch (result) {
        case bup::BackupMemoryImageLoadResult::Success:
            settings.imagePath = file;
            settings.capacity = SizeToCapacity(bupMem.Size());
            m_context.EnqueueEvent(events::emu::InsertCartridgeFromSettings());
            MakeDirty();
            break;
        case bup::BackupMemoryImageLoadResult::FilesystemError:
            if (error) {
                m_context.EnqueueEvent(
                    events::gui::ShowError(fmt::format("无法加载备份内存镜像：{}", error.message())));
            } else {
                m_context.EnqueueEvent(events::gui::ShowError(
                    fmt::format("无法加载备份内存镜像：未指定的文件系统错误")));
            }
            break;
        case bup::BackupMemoryImageLoadResult::InvalidSize:
            m_context.EnqueueEvent(
                events::gui::ShowError(fmt::format("无法加载备份内存镜像：镜像大小无效")));
            break;
        default:
            m_context.EnqueueEvent(
                events::gui::ShowError(fmt::format("无法加载备份内存镜像：意外错误")));
            break;
        }
    } else {
        // The user wants to create a new image file
        if (file.empty()) {
            file = fmt::format("{}",
                               m_context.profile.GetPath(ProfilePath::PersistentState) /
                                   fmt::format("bup-ext-{}M.bin", CapacityToSize(settings.capacity) * 8 / 1024 / 1024));
        }
        bupMem.CreateFrom(file, false, error, CapacityToBupSize(settings.capacity));
        if (error) {
            m_context.EnqueueEvent(
                events::gui::ShowError(fmt::format("无法加载备份内存镜像：{}", error.message())));
        } else {
            settings.imagePath = file;
            m_context.EnqueueEvent(events::emu::InsertCartridgeFromSettings());
            MakeDirty();
        }
    }
}

void CartridgeSettingsView::ShowLoadBackupImageError(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(fmt::format("无法加载备份内存镜像：{}", message)));
}

void CartridgeSettingsView::ProcessLoadROMImage(void *userdata, std::filesystem::path file, int filter) {
    static_cast<CartridgeSettingsView *>(userdata)->LoadROMImage(file);
}

void CartridgeSettingsView::ProcessLoadROMImageError(void *userdata, const char *message, int filter) {
    static_cast<CartridgeSettingsView *>(userdata)->ShowLoadROMImageError(message);
}

void CartridgeSettingsView::LoadROMImage(std::filesystem::path file) {
    auto &settings = GetSettings().cartridge.rom;

    // TODO: rework this entire process
    // - everything here should be done by the emulator event

    // Check that the file exists
    if (!std::filesystem::is_regular_file(file)) {
        m_context.EnqueueEvent(
            events::gui::ShowError(fmt::format("无法加载 ROM 卡带镜像：{} 不是文件", file)));
        return;
    }

    // Check that the file has contents
    const auto size = std::filesystem::file_size(file);
    if (size == 0) {
        m_context.EnqueueEvent(
            events::gui::ShowError("无法加载 ROM 卡带镜像：文件为空或无法读取。"));
        return;
    }

    // Check that the image is not larger than the ROM cartridge capacity
    if (size > cart::kROMCartSize) {
        m_context.EnqueueEvent(events::gui::ShowError(fmt::format(
            "无法加载 ROM 卡带镜像：文件太大（{} > {} 字节）", size, cart::kROMCartSize)));
        return;
    }

    // TODO: Check that the image is a proper Sega Saturn cartridge (headers)

    // Update settings and insert cartridge
    settings.imagePath = file;
    m_context.EnqueueEvent(events::emu::InsertCartridgeFromSettings());
    MakeDirty();
}

void CartridgeSettingsView::ShowLoadROMImageError(const char *message) {
    m_context.EnqueueEvent(events::gui::ShowError(fmt::format("无法加载 ROM 卡带镜像：{}", message)));
}

} // namespace app::ui
