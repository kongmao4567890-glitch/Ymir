#include "about_window.hpp"

#include <ymir/version.hpp>

#include <util/std_lib.hpp>
#include <ymir/util/compiler_info.hpp>

#include <ymir/hw/vdp/vdp.hpp>

#include <app/services/graphics_service.hpp>
#include <app/services/midi_service.hpp>

#include <app/ui/fonts/IconsMaterialSymbols.h>

#include <SDL3/SDL_clipboard.h>

// Includes for versions only
#include <SDL3/SDL.h>
#include <curl/curlver.h>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <lz4.h>
#include <nghttp2/nghttp2ver.h>
#include <nghttp3/version.h>
#include <ngtcp2/version.h>
#include <nlohmann/json.hpp>
#include <openssl/opensslv.h>
#include <rtmidi/RtMidi.h>
#include <semver.hpp>
#include <toml++/toml.hpp>
#include <xxhash.h>
#include <zlib.h>
#include <zstd.h>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <miniz/miniz.h>

#define _STR_IMPL(x) #x
#define _STR(x) _STR_IMPL(x)
#define _SEMVER_STR(major, minor, patch) _STR(major.minor.patch)

#define BROTLI_VERSION "1.1.0" // Not exported
#define CEREAL_VERSION "1.3.2" // Not exported
#define CMRC_VERSION "2.0.0"   // Not exported
#define CURL_VERSION _SEMVER_STR(LIBCURL_VERSION_MAJOR, LIBCURL_VERSION_MINOR, LIBCURL_VERSION_PATCH)
#define CXXOPTS_VERSION _SEMVER_STR(CXXOPTS__VERSION_MAJOR, CXXOPTS__VERSION_MINOR, CXXOPTS__VERSION_PATCH)
#define DATE_VERSION "3.0.4" // Not exported
#define IMGUI_VERSION_FULL IMGUI_VERSION " (" _STR(IMGUI_VERSION_NUM) ")"
#define LZMA_VERSION "25.01"    // Private dependency of libchdr
#define LIBCHDR_VERSION "0.3.0" // Not exported
#define MIO_VERSION "1.1.0"     // Not exported
// #define MZ_VERSION "3.1.1"      // Private dependency of libchdr
#define NLOHMANN_JSON_VERSION \
    _SEMVER_STR(NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH)
#define SDL_VERSION_STR _SEMVER_STR(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION)
#define SEMVER_VERSION _SEMVER_STR(SEMVER_VERSION_MAJOR, SEMVER_VERSION_MINOR, SEMVER_VERSION_PATCH)
#define STB_IMAGE_VERSION "2.30"       // Not exported
#define STB_IMAGE_WRITE_VERSION "1.16" // Not exported
#define MC_CONCQUEUE_VERSION "1.0.4"   // Not exported
#define TOMLPP_VERSION _SEMVER_STR(TOML_LIB_MAJOR, TOML_LIB_MINOR, TOML_LIB_PATCH)
#define XXHASH_VERSION _SEMVER_STR(XXH_VERSION_MAJOR, XXH_VERSION_MINOR, XXH_VERSION_RELEASE)

static const std::string fmtVersion = std::to_string(FMT_VERSION / 10000) + "." +
                                      std::to_string(FMT_VERSION / 100 % 100) + "." + std::to_string(FMT_VERSION % 100);

namespace app::ui {

struct License {
    const char *name;
    const char *url;
};

struct FontInfo {
    ImFont *font;
    float size;
};

struct FontDesc {
    using FontFn = FontInfo (*)(SharedContext &ctx);

    const char *name;
    const License &license;
    const char *url;
    FontFn fontFn;
    const char *demoText;
};

// clang-format off
inline constexpr License licenseApache2_0   {.name = "Apache-2.0",    .url = "https://opensource.org/licenses/Apache-2.0"};
inline constexpr License licenseBSD2        {.name = "BSD-2-Clause",  .url = "https://opensource.org/licenses/BSD-2-Clause"};
inline constexpr License licenseBSD3        {.name = "BSD-3-Clause",  .url = "https://opensource.org/licenses/BSD-3-Clause"};
//inline constexpr License licenseBSL         {.name = "BSL-1.0", .      url = "https://opensource.org/license/bsl-1-0"};
//inline constexpr License licenseISC         {.name = "ISC",           .url = "https://opensource.org/licenses/ISC"};
inline constexpr License licenseMIT         {.name = "MIT",           .url = "https://opensource.org/licenses/MIT"};
inline constexpr License licenseMITCurl     {.name = "MIT-cURL",      .url = "https://github.com/curl/curl/blob/master/COPYING"};
inline constexpr License licenseMITRtMidi   {.name = "MIT-RtMidi",    .url = "https://github.com/thestk/rtmidi/blob/master/LICENSE"};
inline constexpr License licensePublicDomain{.name = "公有领域", .url = nullptr};
//inline constexpr License licenseUnlicense   {.name = "Unlicense",     .url = "https://opensource.org/licenses/unlicense"};
inline constexpr License licenseZlib        {.name = "Zlib",          .url = "https://opensource.org/licenses/Zlib"};
inline constexpr License licenseOFL         {.name = "OFL-1.1",       .url = "https://opensource.org/licenses/OFL-1.1"};

static const struct {
    const char *name;
    const char *version = nullptr;
    const License &license;
    const char *repoURL;
    const char *licenseURL = nullptr;
    const bool repoPrivate = false;
    const char *homeURL = nullptr;
} depsCode[] = {
    {.name = "Brotli",                        .version = BROTLI_VERSION,             .license = licenseMIT,           .repoURL = "https://github.com/google/brotli",               .licenseURL = "https://github.com/google/brotli/blob/master/LICENSE"},
    {.name = "cereal",                        .version = CEREAL_VERSION,             .license = licenseBSD3,          .repoURL = "https://github.com/USCiLab/cereal",              .licenseURL = "https://github.com/USCiLab/cereal/blob/master/LICENSE",                  .homeURL = "https://uscilab.github.io/cereal/index.html"},
    {.name = "CMakeRC",                       .version = CMRC_VERSION,               .license = licenseMIT,           .repoURL = "https://github.com/vector-of-bool/cmrc",         .licenseURL = "https://github.com/vector-of-bool/cmrc/blob/master/LICENSE.txt"},
    {.name = "curl",                          .version = CURL_VERSION,               .license = licenseMITCurl,       .repoURL = "https://github.com/curl/curl",                   .licenseURL = "https://github.com/curl/curl/blob/master/COPYING",                       .homeURL = "https://curl.se/"},
    {.name = "cxxopts",                       .version = CXXOPTS_VERSION,            .license = licenseMIT,           .repoURL = "https://github.com/jarro2783/cxxopts",           .licenseURL = "https://github.com/jarro2783/cxxopts/blob/master/LICENSE"},
    {.name = "date",                          .version = DATE_VERSION,               .license = licenseMIT,           .repoURL = "https://github.com/HowardHinnant/date",          .licenseURL = "https://github.com/HowardHinnant/date/blob/master/LICENSE.txt"},
    {.name = "Dear ImGui",                    .version = IMGUI_VERSION_FULL,         .license = licenseMIT,           .repoURL = "https://github.com/ocornut/imgui",               .licenseURL = "https://github.com/ocornut/imgui/blob/master/LICENSE.txt"},
    {.name = "{fmt}",                         .version = fmtVersion.c_str(),         .license = licenseMIT,           .repoURL = "https://github.com/fmtlib/fmt",                  .licenseURL = "https://github.com/fmtlib/fmt/blob/master/LICENSE",                      .homeURL = "https://fmt.dev/latest/index.html"},
    {.name = "ImGui Club",                                                           .license = licenseMIT,           .repoURL = "https://github.com/ocornut/imgui_club",          .licenseURL = "https://github.com/ocornut/imgui_club/blob/main/LICENSE.txt"},
    {.name = "libchdr",                       .version = LIBCHDR_VERSION,            .license = licenseBSD3,          .repoURL = "https://github.com/rtissera/libchdr",            .licenseURL = "https://github.com/rtissera/libchdr/blob/master/LICENSE.txt"},
    {.name = "lz4",                           .version = LZ4_VERSION_STRING,         .license = licenseBSD2,          .repoURL = "https://github.com/lz4/lz4",                     .licenseURL = "https://github.com/lz4/lz4/blob/dev/lib/LICENSE",                        .homeURL = "https://lz4.org/",},
    {.name = "lzma",                          .version = LZMA_VERSION,               .license = licensePublicDomain,                                                                                                                                                       .homeURL = "https://www.7-zip.org/sdk.html",},
    {.name = "mio",                           .version = MIO_VERSION,                .license = licenseMIT,           .repoURL = "https://github.com/StrikerX3/mio",               .licenseURL = "https://github.com/StrikerX3/mio/blob/master/LICENSE"},
    {.name = "miniz",                         .version = MZ_VERSION,                 .license = licenseMIT,           .repoURL = "https://github.com/richgel999/miniz",            .licenseURL = "https://github.com/richgel999/miniz/blob/master/LICENSE"},
    {.name = "moodycamel::\nConcurrentQueue", .version = "\n" MC_CONCQUEUE_VERSION,  .license = licenseBSD2,          .repoURL = "https://github.com/cameron314/concurrentqueue",  .licenseURL = "https://github.com/cameron314/concurrentqueue/blob/master/LICENSE.md"},
    {.name = "Neargye/semver",                .version = SEMVER_VERSION,             .license = licenseMIT,           .repoURL = "https://github.com/Neargye/semver",              .licenseURL = "https://github.com/Neargye/semver/blob/master/LICENSE"},
    {.name = "nghttp2",                       .version = NGHTTP2_VERSION,            .license = licenseMIT,           .repoURL = "https://github.com/nghttp2/nghttp2",             .licenseURL = "https://github.com/nghttp2/nghttp2/blob/master/COPYING",                 .homeURL = "https://nghttp2.org/"},
    {.name = "nghttp3",                       .version = NGHTTP3_VERSION,            .license = licenseMIT,           .repoURL = "https://github.com/ngtcp2/nghttp3",              .licenseURL = "https://github.com/ngtcp2/nghttp3/blob/main/COPYING",                    .homeURL = "https://nghttp2.org/nghttp3/"},
    {.name = "ngtcp2",                        .version = NGTCP2_VERSION,             .license = licenseMIT,           .repoURL = "https://github.com/ngtcp2/ngtcp2",               .licenseURL = "https://github.com/ngtcp2/ngtcp2/blob/main/COPYING",                     .homeURL = "https://nghttp2.org/ngtcp2/"},
    {.name = "nlohmann/json",                 .version = NLOHMANN_JSON_VERSION,      .license = licenseMIT,           .repoURL = "https://github.com/nlohmann/json",               .licenseURL = "https://github.com/nlohmann/json/blob/develop/LICENSE.MIT",              .homeURL = "https://json.nlohmann.me/"},
    {.name = "OpenSSL",                       .version = OPENSSL_FULL_VERSION_STR,   .license = licenseApache2_0,     .repoURL = "https://github.com/openssl/openssl",             .licenseURL = "https://github.com/openssl/openssl/blob/master/LICENSE.txt",             .homeURL = "https://www.openssl.org/"},
    {.name = "RtMidi",                        .version = RTMIDI_VERSION,             .license = licenseMITRtMidi,     .repoURL = "https://github.com/thestk/rtmidi",               .licenseURL = "https://github.com/thestk/rtmidi/blob/master/LICENSE"},
    {.name = "SDL3",                          .version = SDL_VERSION_STR,            .license = licenseZlib,          .repoURL = "https://github.com/libsdl-org/SDL",              .licenseURL = "https://github.com/libsdl-org/SDL/blob/main/LICENSE.txt"},
    {.name = "SDL_GameControllerDB",                                                 .license = licenseZlib,          .repoURL = "https://github.com/mdqinc/SDL_GameControllerDB", .licenseURL = "https://github.com/mdqinc/SDL_GameControllerDB/blob/master/LICENSE"},
    {.name = "stb_image",                     .version = STB_IMAGE_VERSION,          .license = licenseMIT,           .repoURL = "https://github.com/nothings/stb",                .licenseURL = "https://github.com/nothings/stb/blob/master/LICENSE"},
    {.name = "stb_image_write",               .version = STB_IMAGE_WRITE_VERSION,    .license = licenseMIT,           .repoURL = "https://github.com/nothings/stb",                .licenseURL = "https://github.com/nothings/stb/blob/master/LICENSE"},
    {.name = "toml++",                        .version = TOMLPP_VERSION,             .license = licenseMIT,           .repoURL = "https://github.com/marzer/tomlplusplus" ,        .licenseURL = "https://github.com/marzer/tomlplusplus/blob/master/LICENSE",             .homeURL = "https://marzer.github.io/tomlplusplus/"},
    {.name = "xxHash",                        .version = XXHASH_VERSION,             .license = licenseBSD2,          .repoURL = "https://github.com/Cyan4973/xxHash",             .licenseURL = "https://github.com/Cyan4973/xxHash/blob/dev/LICENSE",                    .homeURL = "https://xxhash.com/"},
    {.name = "zlib",                          .version = ZLIB_VERSION,               .license = licenseZlib,          .repoURL = "https://github.com/madler/zlib",                 .licenseURL = "https://github.com/madler/zlib/blob/develop/LICENSE",                    .homeURL = "https://zlib.net/"},
    {.name = "zstd",                          .version = ZSTD_VERSION_STRING,        .license = licenseBSD3,          .repoURL = "https://github.com/facebook/zstd",               .licenseURL = "https://github.com/facebook/zstd/blob/dev/LICENSE",                      .homeURL = "http://www.zstd.net/"},
};


static const char *demoTextStandard =
    "The quick brown fox jumps over the lazy dog\n"
    "0123456789 `~!@#$%^&*()_+-=[]{}<>,./?;:'\"\\|\n"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ  \u00C0\u00C9\u00CE\u00D5\u00DA\u00D1\u00C7\u00DD\n"
    "abcdefghijklmnopqrstuvwxyz  \u00E0\u00E9\u00EE\u00F5\u00FA\u00F1\u00E7\u00FD";

static const char *demoTextStandardJP =
    "いろはにほへとちりぬるを\n"
    "わかよたれそつねならむ\n"
    "うゐのおくやまけふこえて\n"
    "あさきゆめみしゑひもせす\n"
    "\n"
    "吾輩は猫である。名前はまだ無い。どこで生れたかとんと見当がつかつ。\n"
    "何でも薄暗いじめじめした所でニャーニャー泣いていた事だけは記憶している。\n";

static const char *demoTextMaterialSymbols =
    ICON_MS_HOME          ICON_MS_HELP         ICON_MS_FOLDER         ICON_MS_DOCS             ICON_MS_SETTINGS      ICON_MS_MENU         ICON_MS_HISTORY      ICON_MS_HISTORY_OFF    "\n"
    ICON_MS_PLAY_ARROW    ICON_MS_PAUSE        ICON_MS_PLAY_PAUSE     ICON_MS_ARROW_BACK_2     ICON_MS_FAST_FORWARD  ICON_MS_FAST_REWIND  ICON_MS_SKIP_NEXT    ICON_MS_SKIP_PREVIOUS  "\n"
    ICON_MS_VOLUME_MUTE   ICON_MS_VOLUME_UP    ICON_MS_VOLUME_DOWN    ICON_MS_VOLUME_OFF       ICON_MS_NO_SOUND      ICON_MS_TUNE         ICON_MS_EJECT        ICON_MS_ALBUM          "\n"
    ICON_MS_STEP          ICON_MS_STEP_INTO    ICON_MS_STEP_OVER      ICON_MS_STEP_OUT         ICON_MS_BUG_REPORT    ICON_MS_CODE         ICON_MS_MEMORY       ICON_MS_TV             "\n"
    ICON_MS_CONTENT_COPY  ICON_MS_CONTENT_CUT  ICON_MS_CONTENT_PASTE  ICON_MS_VIDEOGAME_ASSET  ICON_MS_JOYSTICK      ICON_MS_GAMEPAD      ICON_MS_MOUSE        ICON_MS_KEYBOARD;

static const FontDesc fontDescs[] = {
    { .name = "Material Symbols", .license = licenseApache2_0, .url = "https://fonts.google.com/icons",               .fontFn = [](SharedContext &ctx) -> FontInfo { return {ctx.fonts.sansSerif.regular, 24.0f}; }, .demoText = demoTextMaterialSymbols },
    { .name = "Spline Sans",      .license = licenseOFL,       .url = "https://github.com/SorkinType/SplineSans",     .fontFn = [](SharedContext &ctx) -> FontInfo { return {ctx.fonts.sansSerif.regular, 16.0f}; }, .demoText = demoTextStandard },
    { .name = "Spline Sans Mono", .license = licenseOFL,       .url = "https://github.com/SorkinType/SplineSansMono", .fontFn = [](SharedContext &ctx) -> FontInfo { return {ctx.fonts.monospace.regular, 16.0f}; }, .demoText = demoTextStandard },
    { .name = "M PLUS U",         .license = licenseOFL,       .url = "https://github.com/coz-m/MPLUS_FONTS",         .fontFn = [](SharedContext &ctx) -> FontInfo { return {ctx.fonts.sansSerif.regular, 16.0f}; }, .demoText = demoTextStandardJP },
    { .name = "Zen Dots",         .license = licenseOFL,       .url = "https://github.com/googlefonts/zen-dots",      .fontFn = [](SharedContext &ctx) -> FontInfo { return {ctx.fonts.display,           24.0f}; }, .demoText = demoTextStandard },
};
// clang-format on

static const std::unordered_map<std::string_view, const char *> kRenderers = {
    {"vulkan", "Vulkan"}, {"direct3d", "Direct3D 9"}, {"direct3d11", "Direct3D 11"}, {"direct3d12", "Direct3D 12"},
    {"metal", "Metal"},   {"opengl", "OpenGL"},       {"opengles2", "OpenGL ES 2"},
};

const char *RendererToHumanReadableString(std::string_view driver) {
    if (kRenderers.contains(driver)) {
        return kRenderers.at(driver);
    }
    return driver.data();
}

// If only SDL3 exposed the nice desc field they already have in the SDL_AudioDriver struct...
// Also note that just because certain systems are listed here, it doesn't mean Ymir actually supports them.
static const std::unordered_map<std::string_view, const char *> kAudioDrivers = {
    {"AAudio", "AAudio 音频驱动"},
    {"alsa", "ALSA PCM 音频"},
    {"coreaudio", "CoreAudio"},
    {"directsound", "DirectSound"},
    {"disk", "直接写入磁盘音频"},
    {"dsp", "Open Sound System (/dev/dsp)"},
    {"dummy", "SDL 虚拟音频驱动"},
    {"emscripten", "SDL emscripten 音频驱动"},
    {"haiku", "Haiku BSoundPlayer"},
    {"jack", "JACK Audio Connection Kit"},
    {"netbsd", "NetBSD 音频"},
    {"N-Gage", "N-Gage 音频驱动"},
    {"n3ds", "SDL N3DS 音频驱动"},
    {"openslES", "OpenSL ES 音频驱动"},
    {"pipewire", "Pipewire"},
    {"psp", "PSP 音频驱动"},
    {"ps2", "PS2 音频驱动"},
    {"pulseaudio", "PulseAudio"},
    {"qsa", "QNX QSA 音频"},
    {"sndio", "OpenBSD sndio"},
    {"vita", "VITA 音频驱动"},
    {"wasapi", "WASAPI"},
};

const char *AudioDriverToHumanReadableString(std::string_view driver) {
    if (kAudioDrivers.contains(driver)) {
        return kAudioDrivers.at(driver);
    }
    return "未知";
}

AboutWindow::AboutWindow(SharedContext &context)
    : WindowBase(context) {

    m_windowConfig.name = "关于";
}

void AboutWindow::PrepareWindow() {
    auto *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + vp->Size.y * 0.5f), ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(660 * m_context.displayScale, 800 * m_context.displayScale),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(400 * m_context.displayScale, 240 * m_context.displayScale),
                                        ImVec2(1000 * m_context.displayScale, 900 * m_context.displayScale));
}

void AboutWindow::DrawContents() {
    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("关于")) {
            if (ImGui::BeginChild("##about")) {
                DrawAboutTab();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("依赖")) {
            if (ImGui::BeginChild("##dependencies")) {
                DrawDependenciesTab();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("致谢")) {
            if (ImGui::BeginChild("##acknowledgements", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
                DrawAcknowledgementsTab();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void AboutWindow::DrawAboutTab() {
    ImGui::PushTextWrapPos(ImGui::GetWindowContentRegionMax().x);

    const auto &midiService = m_context.serviceLocator.GetRequired<services::MIDIService>();
    const auto &graphicsService = m_context.serviceLocator.GetRequired<services::GraphicsService>();
    SDL_Texture *texture = graphicsService.GetSDLTexture(m_context.images.ymirLogo.texture);
    ImGui::Image((ImTextureID)texture, ImVec2(m_context.images.ymirLogo.size.x * m_context.displayScale,
                                              m_context.images.ymirLogo.size.y * m_context.displayScale));

    ImGui::PushFont(m_context.fonts.display, m_context.fontSizes.display);
    ImGui::TextUnformatted("Ymir");
    ImGui::PopFont();
    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.xlarge);
    ImGui::TextUnformatted("版本 " Ymir_VERSION);
    ImGui::PopFont();
#if Ymir_DEV_BUILD
    ImGui::SameLine();
    ImGui::PushFont(m_context.fonts.sansSerif.regular, m_context.fontSizes.xlarge);
    ImGui::TextUnformatted("(开发版)");
    ImGui::PopFont();
#endif

    ImGui::PushFont(m_context.fonts.sansSerif.regular, m_context.fontSizes.large);
    ImGui::TextUnformatted("世嘉土星模拟器");
    ImGui::PopFont();

    if (ImGui::Button("复制版本")) {
        SDL_SetClipboardText(Ymir_VERSION);
    }

    ImGui::NewLine();
    ImGui::Text("使用 %s %s 编译。", compiler::name, compiler::version::string.c_str());
#ifdef Ymir_BUILD_TIMESTAMP
    if (auto buildTimestamp = util::parse8601(Ymir_BUILD_TIMESTAMP)) {
        auto localBuildTime = util::to_local_time(std::chrono::system_clock::time_point(*buildTimestamp));
        ImGui::TextUnformatted(fmt::format("构建时间 {}", localBuildTime).c_str());
    }
#endif
    if constexpr (ymir::version::is_nightly_build) {
        ImGui::TextUnformatted("Nightly 发布通道。");
    } else if constexpr (ymir::version::is_stable_build) {
        ImGui::TextUnformatted("稳定版发布通道。");
    } else if constexpr (ymir::version::is_local_build) {
        ImGui::TextUnformatted("本地开发构建。");
    }

#if defined(__x86_64__) || defined(_M_X64)
    #ifdef Ymir_AVX2
    ImGui::Text("使用 AVX2 指令集。");
    #else
    ImGui::Text("使用 SSE2 指令集。");
    #endif
#elif defined(__aarch64__) || defined(__arm64__)
    ImGui::Text("使用 NEON 指令集。");
#endif

    SDL_PropertiesID rendererProps = SDL_GetRendererProperties(graphicsService.GetRenderer());
    std::string_view rendererName = SDL_GetStringProperty(rendererProps, SDL_PROP_RENDERER_NAME_STRING, "unknown");
    const char *graphicsBackendName = "unknown";
    if (rendererName == "gpu") {
        auto *gpuDevice = static_cast<SDL_GPUDevice *>(
            SDL_GetPointerProperty(rendererProps, SDL_PROP_RENDERER_GPU_DEVICE_POINTER, nullptr));
        if (gpuDevice) {
            const char *gpuDriver = SDL_GetGPUDeviceDriver(gpuDevice);
            graphicsBackendName = RendererToHumanReadableString(gpuDriver);
        } else {
            graphicsBackendName = "SDL GPU";
        }
    } else {
        graphicsBackendName = RendererToHumanReadableString(rendererName);
    }
    ImGui::Text("使用 %s 图形后端进行界面渲染。", graphicsBackendName);
    const auto &vdp = m_context.saturn.GetVDP();
    {
        std::unique_lock lock{m_context.locks.renderer};
        ImGui::Text("使用 %s VDP1/VDP2 渲染器。", vdp.GetRenderer().GetName().data());
    }

    const char *audioDriver = SDL_GetCurrentAudioDriver();
    ImGui::Text("使用 %s 音频驱动。", AudioDriverToHumanReadableString(audioDriver));
    ImGui::Text("使用 %s MIDI API。", RtMidi::getApiDisplayName(midiService.GetInput()->getCurrentApi()).c_str());

    ImGui::NewLine();
    ImGui::TextUnformatted("采用 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("GPLv3", "https://www.gnu.org/licenses/gpl-3.0.en.html");

    ImGui::TextUnformatted("源代码可在 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("https://github.com/StrikerX3/Ymir");

    ImGui::NewLine();
    ImGui::TextUnformatted("加入官方 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("Discord 服务器", "https://discord.gg/NN3A7n5dzn");

    ImGui::TextUnformatted("欢迎在 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("Patreon", "https://www.patreon.com/StrikerX3");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted(" 上支持我的工作。");

    ImGui::PopTextWrapPos();
}

void AboutWindow::DrawDependenciesTab() {
    static constexpr ImGuiTableFlags kTableFlags = ImGuiTableFlags_SizingFixedFit;

    // -----------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("库");
    ImGui::PopFont();

    if (ImGui::BeginTable("libraries", 3, kTableFlags)) {
        ImGui::TableSetupColumn("名称");
        ImGui::TableSetupColumn("许可证");
        ImGui::TableSetupColumn("链接");
        ImGui::TableHeadersRow();

        for (auto &dep : depsCode) {
            ImGui::PushID(dep.name);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
            ImGui::TextUnformatted(dep.name);
            ImGui::PopFont();
            if (dep.version != nullptr) {
                ImGui::SameLine();
                // TODO: don't hardcode colors!
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.77f, 0.8f, 1.0f));
                ImGui::TextUnformatted(dep.version);
                ImGui::PopStyleColor();
            }

            ImGui::TableSetColumnIndex(1);
            if (dep.licenseURL != nullptr) {
                ImGui::TextLinkOpenURL(dep.license.name, dep.licenseURL);
            } else if (dep.license.url != nullptr) {
                ImGui::TextLinkOpenURL(dep.license.name, dep.license.url);
            } else {
                ImGui::TextUnformatted(dep.license.name);
            }

            ImGui::TableSetColumnIndex(2);
            if (dep.repoURL != nullptr) {
                ImGui::TextLinkOpenURL(dep.repoURL);
            }
            if (dep.repoPrivate) {
                ImGui::SameLine();
                // TODO: don't hardcode colors!
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.77f, 0.8f, 1.0f));
                ImGui::TextUnformatted("(私有)");
                ImGui::PopStyleColor();
            }
            if (dep.homeURL != nullptr) {
                ImGui::TextLinkOpenURL(dep.homeURL);
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // -----------------------------------------------------------------------------

    ImGui::Separator();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("字体");
    ImGui::PopFont();

    if (ImGui::BeginTable("fonts", 3, kTableFlags)) {
        ImGui::TableSetupColumn("名称");
        ImGui::TableSetupColumn("许可证");
        ImGui::TableSetupColumn("链接");
        ImGui::TableHeadersRow();

        for (auto &font : fontDescs) {
            ImGui::PushID(font.name);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            auto cursor = ImGui::GetCursorPos();
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
            ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns);
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered() && (ImGui::TableGetColumnFlags(0) & ImGuiTableColumnFlags_IsHovered)) {
                ImGui::BeginTooltip();
                auto [fontPtr, fontSize] = font.fontFn(m_context);
                ImGui::PushFont(fontPtr, fontSize);
                ImGui::TextUnformatted(font.demoText);
                ImGui::PopFont();
                ImGui::EndTooltip();
            }
            ImGui::SetCursorPos(cursor);

            ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
            ImGui::TextUnformatted(font.name);
            ImGui::PopFont();

            ImGui::TableSetColumnIndex(1);
            ImGui::TextLinkOpenURL(font.license.name, font.license.url);

            ImGui::TableSetColumnIndex(2);
            ImGui::TextLinkOpenURL(font.url);
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void AboutWindow::DrawAcknowledgementsTab() {
    ImGui::PushTextWrapPos(ImGui::GetWindowContentRegionMax().x);

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("Ymir 的诞生离不开");
    ImGui::PopFont();

    auto ack = [&](const char *name, const char *url) {
        ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
        ImGui::TextLinkOpenURL(name, url);
        ImGui::PopFont();
    };

    auto ackWithAuthor = [&](const char *name, const char *author, const char *url) {
        ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
        ImGui::TextLinkOpenURL(name, url);
        ImGui::PopFont();

        ImGui::SameLine();

        ImGui::PushFont(m_context.fonts.sansSerif.regular, m_context.fontSizes.medium);
        ImGui::Text("作者：%s", author);
        ImGui::PopFont();
    };

    ackWithAuthor("antime 的简陋世嘉土星页面", "antime", "https://antime.kapsi.fi/sega/");
    ackWithAuthor("硬件研究和信号追踪", "Sergiy Dvodnenko (srg320)",
                  "https://github.com/srg320/Saturn_hw");
    ackWithAuthor("原始研究", "Charles MacDonald",
                  "https://web.archive.org/web/20150119062930/http://cgfm2.emuviews.com/saturn.php");
    {
        ImGui::Indent();
        ack("世嘉土星硬件笔记 (sattech.txt)",
            "https://web.archive.org/web/20140318183509/http://cgfm2.emuviews.com/txt/sattech.txt");
        ack("VDP1 硬件笔记 (vdp1tech.txt)",
            "https://web.archive.org/web/20150106171745/http://cgfm2.emuviews.com/sat/vdp1tech.txt");
        ack("世嘉土星卡带信息 (satcart.txt)",
            "https://web.archive.org/web/20140724061526/http://cgfm2.emuviews.com/sat/satcart.txt");
        ack("EMS Action Replay Plus 笔记 (satar.txt)",
            "https://web.archive.org/web/20140724045721/http://cgfm2.emuviews.com/sat/satar.txt");
        ack("Comms Link 硬件笔记 (comminfo.txt)",
            "https://web.archive.org/web/20140724035829/http://cgfm2.emuviews.com/sat/comminfo.txt");
        ImGui::Unindent();
    }
    ackWithAuthor("Dreamcast 文档集合", "Senryoku",
                  "https://github.com/Senryoku/dreamcast-docs/tree/master/AICA/DOCS");
    {
        ImGui::Indent();
        ackWithAuthor("原始 AICA 研究", "Neill Corlett",
                      "https://raw.githubusercontent.com/Senryoku/dreamcast-docs/refs/heads/master/"
                      "AICA/DOCS/myaica.txt");
        ImGui::Unindent();
    }
    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.medium);
    ImGui::TextUnformatted("CD block 研究");
    ImGui::PopFont();
    {
        ImGui::Indent();
        ackWithAuthor("CD block 改版芯片研究", "Pinchy",
                      "https://segaxtreme.net/threads/finally-made-a-working-modchip.14781/");
        ackWithAuthor("CD 接口和信号追踪", "Pinchy",
                      "https://web.archive.org/web/20111203080908/http://www.crazynation.org/SEGA/Saturn/cd_tech.htm");
        ackWithAuthor(
            "CD 驱动器命令日志", "Pinchy",
            "https://web.archive.org/web/20111011104440/http://www.crazynation.org/SEGA/Saturn/files/command_log.txt");
        ackWithAuthor(
            "原始光盘格式", "Joachim Metz",
            "https://github.com/libyal/libodraw/blob/main/documentation/Optical%20disc%20RAW%20format.asciidoc");
        ackWithAuthor("YGR 寄存器、ROM 反汇编和信号追踪", "Sergiy Dvodnenko (srg320)",
                      "https://github.com/srg320/Saturn_hw/tree/main/CDB");
        ImGui::Unindent();
    }
    ack("Yabause wiki", "http://wiki.yabause.org/");
    ack("SegaRetro 上的世嘉土星", "https://segaretro.org/Sega_Saturn");

    // -----------------------------------------------------------------------------

    ImGui::NewLine();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("有用的工具和测试套件");
    ImGui::PopFont();

    ackWithAuthor("libyaul", "mrkotfw and contributors", "https://github.com/yaul-org/libyaul");
    ackWithAuthor("libyaul-examples", "mrkotfw and contributors", "https://github.com/yaul-org/libyaul-examples");
    ackWithAuthor("saturn-tests", "StrikerX3", "https://github.com/StrikerX3/saturn-tests");
    ackWithAuthor("SH-4 单步测试", "raddad772", "https://github.com/SingleStepTests/sh4");
    ackWithAuthor("M68000 单步测试", "raddad772", "https://github.com/SingleStepTests/m68000");
    ackWithAuthor("各种测试", "celeriyacon", "https://github.com/celeriyacon");
    ImGui::Indent();
    ImGui::TextUnformatted("cdbtest, memtimes, misctest, scspadpcm, scsptest, scutest, sh2test, smpctest 和 vdp2test");
    ImGui::Unindent();

    // -----------------------------------------------------------------------------

    ImGui::NewLine();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("启发 Ymir 的其他模拟器");
    ImGui::PopFont();

    ackWithAuthor("Saturn MiSTer", "Sergiy Dvodnenko (srg320)", "https://github.com/MiSTer-devel/Saturn_MiSTer");

    ackWithAuthor("Mednafen", "various contributors", "https://mednafen.github.io/");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("(libretro git 镜像)##mednafen", "https://github.com/libretro-mirrors/mednafen-git");

    ackWithAuthor("Yaba Sanshiro 2", "devmiyax", "https://github.com/devmiyax/yabause");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("(网站)##yaba_sanshiro_2", "https://www.uoyabause.org/");

    ackWithAuthor("Yabause", "Guillaume Duhamel and contributors", "https://github.com/Yabause/yabause");

    ackWithAuthor("Mesen2", "Sour and contributors", "https://github.com/SourMesen/Mesen2");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("(网站)##mesen", "https://www.mesen.ca/");

    ackWithAuthor("openMSX", "openMSX developers", "https://github.com/openMSX/openMSX");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("(网站)##openmsx", "https://openmsx.org/");

    ackWithAuthor("DuckStation", "Stenzek and contributors", "https://github.com/stenzek/duckstation");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("(网站)##duckstation", "https://www.duckstation.org/");

    // -----------------------------------------------------------------------------

    ImGui::NewLine();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("特别感谢");
    ImGui::PopFont();

    ImGui::TextUnformatted("感谢 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("/r/EmuDev 社区", "https://www.reddit.com/r/EmuDev/");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted(" 及其 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("Discord 服务器", "https://discord.gg/dkmJAes");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted("。");

    ImGui::TextUnformatted("感谢 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("项目贡献者", "https://github.com/StrikerX3/Ymir/graphs/contributors");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted(" 以及 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("报告问题和功能需求的用户", "https://github.com/StrikerX3/Ymir/issues");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted("，包括：");
    ImGui::Indent();
    ImGui::TextUnformatted("BlueInterlude, "
                           "bsdcode, "
                           "Citrodata, "
                           "floreal, "
                           "Fueziwa, "
                           "GlaireDaggers, "
                           "lvsweat, "
                           "mmkzer0, "
                           "PringleElUno, "
                           "ronan22, "
                           "SternXD, "
                           "tegaidogun, "
                           "tordona, "
                           "Wunkolo.");
    ImGui::Unindent();

    ImGui::TextUnformatted("感谢 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("Ymir 官方 Discord 服务器", "https://discord.gg/NN3A7n5dzn");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted("中的朋友们，特别是：");
    ImGui::Indent();
    ImGui::TextUnformatted("Aydan Watkins, "
                           "celeriyacon, "
                           "Charles / thelastangryman1907, "
                           "Damian Gracz, "
                           "fathamburger, "
                           "GoodWall_533, "
                           "Jano, "
                           "Katanchiro, "
                           "Lordus, "
                           "Reaven, "
                           "sasori95 / Immersion95, "
                           "secreto7, "
                           "Silanda, "
                           "Sorer, "
                           "SternXD, "
                           "TheCoolPup, "
                           "waspennator, "
                           "Zet-sensei.");
    ImGui::Unindent();

    ImGui::TextUnformatted("感谢当前和曾经的 ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("Patreon 支持者", "https://www.patreon.com/StrikerX3");
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted("：");
    ImGui::Indent();
    ImGui::TextUnformatted("Aitor Guevara, "
                           "Armonte, "
                           "Aydan Watkins, "
                           "Chase Heathcliff, "
                           "Derek Fagan, "
                           "Diego Bartolom\u00E9, "
                           "Elcorsico 28, "
                           "Giovani Avelar, "
                           "Israel Jacquez, "
                           "James Wood, "
                           "Jeff Greulich, "
                           "Joek, "
                           "Julien P, "
                           "KC, "
                           "khalifax10, "
                           "Mario Fonseca, "
                           "Mored4u, "
                           "Munch, "
                           "Oliver Stadler, "
                           "Phillip O'Toole, "
                           "rifter, "
                           "Rustle, "
                           "Some Guy, "
                           "TheCoolPup, "
                           "Zrat, "
                           "アレ・.");
    ImGui::Unindent();

    ImGui::PushFont(m_context.fonts.sansSerif.bold, m_context.fontSizes.large);
    ImGui::TextUnformatted("还有你！");
    ImGui::PopFont();

    ImGui::PopTextWrapPos();
}

} // namespace app::ui
