#include "window_manager_service.hpp"

#include <app/services/file_dialog_service.hpp>
#include <app/services/graphics_service.hpp>
#include <app/services/rom_service.hpp>

#include <SDL3/SDL.h>
#include <chrono>
#include <fmt/format.h>
#include <imgui.h>

namespace app::services {

WindowManagerService::WindowManagerService(SharedContext &context, Settings &settings, CheatManager &cheatManager)
    : m_context(context)
    , m_settings(settings)
    , m_cheatManager(cheatManager)
    , m_systemStateWindow(m_context)
    , m_bupMgrWindow(m_context)
    , m_cheatManagerWindow(m_context, m_cheatManager)
    , m_masterSH2WindowSet(m_context, true)
    , m_slaveSH2WindowSet(m_context, false)
    , m_scuWindowSet(m_context)
    , m_scspWindowSet(m_context)
    , m_vdpWindowSet(m_context)
    , m_cdblockWindowSet(m_context)
    , m_debugOutputWindow(m_context)
    , m_settingsWindow(m_context)
    , m_periphConfigWindow(m_context)
    , m_messageHistoryWindow(m_context)
    , m_aboutWindow(m_context)
    , m_updateOnboardingWindow(m_context)
    , m_updateWindow(m_context) {

    // Preinitialize some memory viewers
    for (int i = 0; i < 8; i++) {
        m_memoryViewerWindows.emplace_back(m_context);
    }
}

void WindowManagerService::DrawWindows() {
    m_systemStateWindow.Display();
    m_bupMgrWindow.Display();
    m_cheatManagerWindow.Display();

    m_masterSH2WindowSet.DisplayAll();
    m_slaveSH2WindowSet.DisplayAll();
    m_scuWindowSet.DisplayAll();
    m_scspWindowSet.DisplayAll();
    m_vdpWindowSet.DisplayAll();
    m_cdblockWindowSet.DisplayAll();

    m_debugOutputWindow.Display();

    for (auto &memView : m_memoryViewerWindows) {
        memView.Display();
    }

    m_settingsWindow.Display();
    m_periphConfigWindow.Display();
    m_messageHistoryWindow.Display();
    m_aboutWindow.Display();
#if Ymir_ENABLE_UPDATE_CHECKS
    m_updateOnboardingWindow.Display();
#endif
    m_updateWindow.Display();
}

void WindowManagerService::OpenMemoryViewer() {
    for (auto &memView : m_memoryViewerWindows) {
        if (!memView.Open) {
            memView.Open = true;
            memView.RequestFocus();
            return;
        }
    }

    // If there are no more free memory viewers, request focus on the first window
    m_memoryViewerWindows[0].RequestFocus();
}

void WindowManagerService::OpenPeripheralBindsEditor(uint32 portIndex, uint32 slotIndex) {
    m_periphConfigWindow.Open(portIndex, slotIndex);
    m_periphConfigWindow.RequestFocus();
}

void WindowManagerService::DrawGenericModal() {
    std::string title = fmt::format("{}##generic_modal", m_genericModalTitle);

    if (m_openGenericModal) {
        m_openGenericModal = false;
        ImGui::OpenPopup(title.c_str());
    }

    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushTextWrapPos(500.0f * m_context.displayScale);
        if (m_genericModalContents) {
            m_genericModalContents();
        }

        ImGui::PopTextWrapPos();

        bool close = m_closeGenericModal;
        if (m_showOkButtonInGenericModal) {
            if (ImGui::Button("确定", ImVec2(80 * m_context.displayScale, 0 * m_context.displayScale))) {
                close = true;
            }
        }
        if (close) {
            ImGui::CloseCurrentPopup();
            m_genericModalContents = {};
            m_closeGenericModal = false;
        }

        ImGui::EndPopup();
    }
}

void WindowManagerService::OpenSimpleErrorModal(std::string message) {
    OpenGenericModal("Error", [=] { ImGui::Text("%s", message.c_str()); });
}

void WindowManagerService::OpenGenericModal(std::string title, std::function<void()> fnContents, bool showOKButton) {
    m_openGenericModal = true;
    m_genericModalTitle = title;
    m_genericModalContents = fnContents;
    m_showOkButtonInGenericModal = showOKButton;
}

void WindowManagerService::CloseGenericModal() {
    m_closeGenericModal = true;
}

void WindowManagerService::OpenWelcomeModal(bool scanIPLROMs) {
    bool activeScanning = scanIPLROMs;

    using namespace std::chrono_literals;
    static constexpr auto kScanInterval = 400ms;
    using clk = std::chrono::steady_clock;

    struct ROMSelectResult {
        bool fileSelected = false;
        std::filesystem::path path;

        bool hasResult = false;
        util::ROMLoadResult result;
    };

    OpenGenericModal("欢迎", [=, this, nextScanDeadline = clk::now() + kScanInterval,
                                 lastROMCount = m_context.romManager.GetIPLROMs().size(),
                                 romSelectResult = ROMSelectResult{}]() mutable {
        bool doSelectRom = false;
        bool doOpenSettings = false;

        auto graphicsService = m_context.serviceLocator.Get<GraphicsService>();
        SDL_Texture *logoTexture =
            graphicsService ? graphicsService->GetSDLTexture(m_context.images.ymirLogo.texture) : nullptr;
        if (logoTexture) {
            ImGui::Image((ImTextureID)logoTexture,
                         ImVec2(m_context.images.ymirLogo.size.x * m_context.displayScale * 0.7f,
                                m_context.images.ymirLogo.size.y * m_context.displayScale * 0.7f));
        }

        ImGui::PushFont(m_context.fonts.display,
                        m_context.fontSizes.display); // Fallback if Display size is not separate in ImFont
        ImGui::TextUnformatted("Ymir");
        ImGui::PopFont();
        ImGui::PushFont(m_context.fonts.sansSerif.regular, m_context.fontSizes.large);
        ImGui::TextUnformatted("欢迎使用 Ymir！");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextUnformatted("Ymir 需要有效的 IPL (BIOS) ROM 才能运行。");

        ImGui::NewLine();
        ImGui::TextUnformatted("Ymir 会自动加载放置在以下目录中的 IPL ROM：");
        ImGui::SameLine(0, 0);
        auto romPath = m_context.profile.GetPath(ProfilePath::IPLROMImages);
        if (ImGui::TextLink(fmt::format("{}", romPath).c_str())) {
            SDL_OpenURL(fmt::format("file:///{}", romPath).c_str());
        }
        ImGui::TextUnformatted("你也可以");
        ImGui::SameLine(0, 0);
        if (ImGui::TextLink("手动选择 IPL ROM")) {
            doSelectRom = true;
        }
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted("，或在");
        ImGui::SameLine(0, 0);
        if (ImGui::TextLink("设置 > IPL 中管理 ROM 设置")) {
            doOpenSettings = true;
        }
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted("。");

        if (!activeScanning) {
            ImGui::NewLine();
            ImGui::TextUnformatted("Ymir 当前未在扫描 IPL ROM。");
            ImGui::TextUnformatted("如果你想主动扫描 IPL ROM，请点击下方按钮。");
            if (ImGui::Button("开始主动扫描")) {
                activeScanning = true;
            }
            ImGui::NewLine();
        }

        if (romSelectResult.hasResult && !romSelectResult.result.succeeded) {
            ImGui::NewLine();
            ImGui::Text("文件 %s 不包含有效的 IPL ROM。",
                        fmt::format("{}", romSelectResult.path).c_str());
            ImGui::Text("原因：%s。", romSelectResult.result.errorMessage.c_str());
        }

        ImGui::Separator();

        if (ImGui::Button("打开 IPL ROM 目录")) {
            SDL_OpenURL(fmt::format("file:///{}", m_context.profile.GetPath(ProfilePath::IPLROMImages)).c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("选择 IPL ROM...")) {
            doSelectRom = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("打开 IPL 设置")) {
            doOpenSettings = true;
        }
        ImGui::SameLine(); // this places the OK button next to these

        auto fileDialogService = m_context.serviceLocator.Get<FileDialogService>();
        auto romService = m_context.serviceLocator.Get<ROMService>();

        if (doSelectRom && fileDialogService) {
            FileDialogParams params{
                .dialogTitle = "选择 IPL ROM",
                .defaultPath = m_context.profile.GetPath(ProfilePath::IPLROMImages),
                .filters = {{"ROM files (*.bin, *.rom)", "bin;rom"}, {"All files (*.*)", "*"}},
                .userdata = &romSelectResult,
                .callback =
                    [](void *userdata, const char *const *filelist, int filter) {
                        if (filelist == nullptr) {
                            devlog::error<grp::base>("Failed to open file dialog: {}", SDL_GetError());
                        } else if (*filelist == nullptr) {
                            devlog::info<grp::base>("File dialog cancelled");
                        } else {
                            // Only one file should be selected
                            const char *file = *filelist;
                            std::string fileStr = file;
                            std::u8string u8File{fileStr.begin(), fileStr.end()};
                            auto &result = *static_cast<ROMSelectResult *>(userdata);
                            result.fileSelected = true;
                            result.path = u8File;
                        }
                    },
            };
            fileDialogService->InvokeOpenFileDialog(params);
        }

        if (doOpenSettings) {
            m_settingsWindow.OpenTab(ui::SettingsTab::IPL);
            m_closeGenericModal = true;
        }

        // Try loading IPL ROM selected through the file dialog.
        // If successful, set the override path and close the modal.
        if (romSelectResult.fileSelected && romService) {
            romSelectResult.fileSelected = false;
            romSelectResult.hasResult = true;
            romSelectResult.result = util::LoadIPLROM(romSelectResult.path, *m_context.saturn.instance);
            if (romSelectResult.result.succeeded) {
                m_settings.system.ipl.overrideImage = true;
                m_settings.system.ipl.path = romSelectResult.path;
                romService->LoadIPLROM();
                m_closeGenericModal = true;
            }
        }

        if (!activeScanning) {
            return;
        }

        // Periodically scan for IPL ROMs.
        if (clk::now() >= nextScanDeadline && romService) {
            nextScanDeadline = clk::now() + kScanInterval;

            romService->ScanIPLROMs();

            // Don't load until files stop being added to the directory
            auto romCount = m_context.romManager.GetIPLROMs().size();
            if (romCount != lastROMCount) {
                lastROMCount = romCount;
            } else {
                util::ROMLoadResult result = romService->LoadIPLROM();
                if (result.succeeded) {
                    m_context.EnqueueEvent(events::emu::HardReset());
                    m_closeGenericModal = true;
                }
            }
        }
    });
}

} // namespace app::services
