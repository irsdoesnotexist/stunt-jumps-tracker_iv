#pragma once

#include <Windows.h>

#include <stdint.h>
#include <memory>


namespace sjt4 {
int64_t __stdcall mainWndProc (HWND, uint32_t, uint64_t, int64_t);
static constexpr const char* const mainWndClsName {"sjt4MainWnd"};
static constexpr const char* const appName      {"Stunt Jumps Tracker IV"};
static constexpr const char* const shortAppName {"sjt4"};

namespace app {


inline HWND mainWindow;

bool init(const HINSTANCE) noexcept;


}
}