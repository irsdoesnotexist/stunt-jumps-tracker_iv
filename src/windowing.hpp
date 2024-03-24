#pragma once
#include "app.hpp"

#include <um/Windows.h>

#include <cstdint>
#include <memory>
#include <span>

namespace sjt4_wndwng {

extern "C" typedef struct SJT4_wndInfo {
    std::uint32_t     dwExStyle;
    LPCWSTR           lpClassName;
    LPCWSTR           lpWindowName;
    std::uint32_t     dwStyle;
    std::int32_t      x;
    std::int32_t      y;
    std::int32_t      nWidth;
    std::int32_t      nHeight;
    const HWND        hParent; //win32 handle
    HMENU             hMenu;   //void*
    HINSTANCE         hInstance;//void*
    void*             lpParam;
} SJT4_wndInfo_t;

class wndClass_t {  // Manages lifetime of win32 window classes
private:
    LPCWSTR  name;
    HINSTANCE app;
public:
    wndClass_t(const WNDCLASS&, const HINSTANCE) noexcept;
    ~wndClass_t() noexcept;

    static std::shared_ptr<wndClass_t> clsCheckNRegister(const WNDCLASS&, const HINSTANCE) noexcept;
} __declspec(unused);

class Wnd_t {
private:
    HWND handle;
    std::shared_ptr<wndClass_t> p_cls;
public:
    Wnd_t(const SJT4_wndInfo_t& wndInfo, const WNDCLASS&) noexcept;
};
//
}