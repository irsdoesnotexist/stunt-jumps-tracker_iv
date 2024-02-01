#include <um/Windows.h>

#include <cstdint>
#include <span>
#include <unordered_set>
#include <initializer_list>

#include "windowing.hpp"
#include "renderer.hpp"

#ifndef SJT4_APP_H
#define SJT4_APP_H


#define APP_NAME_SHORT "sjt4"
#define APP_NAME       "Stunt Jumps Tracker 4"

namespace sjt4_app {

class App_t {
private:
    static sjt4_rndr::  Rndr_t vkrenderer;
    static sjt4_wndwng::Wnd_t  mainWnd;
    #if defined (_WIN32)
    static std::unordered_set<char*> wndClassesNames;

    #endif
    
    App_t() = default;
public:
    App_t(App_t&) = delete;
    static App_t instance;

    bool init(std::span<const WNDCLASSA&>) noexcept;

    static std::int32_t* __declspec(stdcall) wndProc_mainWndCls(HWND, std::uint32_t msgbits, std::uint32_t* wParam, std::int32_t* lParam) noexcept;

    ~App_t() = default;
};



}
#endif