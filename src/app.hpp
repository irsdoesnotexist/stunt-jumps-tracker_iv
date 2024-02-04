#pragma once

#include <um/Windows.h>

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "windowing.hpp"

#define SJT4_WINDOW_CLASS_MAIN_WINDOW_CLASS_NAME "sjt4_mainWndCls"

namespace sjt4_app {
extern inline constexpr char g_appName[] {"Stunt Jumps Tracker IV"};
extern inline constexpr char g_shortName[] {"sjt4"};

class App_t {
private:
    App_t(HINSTANCE);

    static HINSTANCE appInst;
    static sjt4_wndwng::Wnd_t startWnd;
public:
    App_t(App_t&) = delete;
    static App_t instance;
    
    static std::int32_t* __declspec(stdcall) mainWndProc (HWND, std::uint32_t msgType, std::uint32_t* wParam, std::int32_t* lParam) noexcept;
    static std::unordered_map<LPCWSTR, std::weak_ptr<sjt4_wndwng::wndClass_t>> wndClasses;

    static bool init();

    static HINSTANCE hinstance() noexcept __declspec(pure);
};

}

#define SJT4_MAIN_WINDOW_CREATE_INFO_EX                                         \
    sjt4_wndwng::SJT4_wndInfo_t{                                                \
        .dwExStyle {NULL|WS_EX_ACCEPTFILES|WS_EX_APPWINDOW|WS_EX_TOPMOST},      \
        .lpClassName {SJT4_WINDOW_CLASS_MAIN_WINDOW_CLASS_NAME},                \
        .lpWindowName{sjt4_app::g_appName},                                     \
        .x{CW_USEDEFAULT}, .y{CW_USEDEFAULT},                                   \
        .nWidth{CW_USEDEFAULT}, .hHeight{CW_USEDEFAULT},                        \
        .hParent{nullptr}, .hMenu{nullptr},                                     \
        .hInstance{sjt4_app::App_t::hinstance()},                               \
        .lpParam{nullptr}                                                       \
    }
