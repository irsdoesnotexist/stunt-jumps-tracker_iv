#include "app.hpp"
#include "renderer.hpp"

#include <Windows.h>

#include <processenv.h>
#include <io.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

int32_t __stdcall WinMain(HINSTANCE hInst, HINSTANCE, char*, int32_t cmdShow) {


#ifdef DEBUG
    AllocConsole();
    HANDLE const h_out {GetStdHandle(STD_OUTPUT_HANDLE)};
    const int hCrt { _open_osfhandle(reinterpret_cast<intptr_t>(h_out), _O_TEXT) };
    FILE* const hf_out { _fdopen(hCrt, "w") };
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;
#endif

    sjt4::app::init(hInst);
    sjt4::rndr::init(sjt4::app::mainWindow, hInst);
    ShowWindow(sjt4::app::mainWindow, cmdShow);
    MSG msg;
    while (GetMessage(&msg, nullptr, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    cleanup:

    return 0;
}

int64_t __stdcall sjt4::mainWndProc (HWND wnd, uint32_t msg, uint64_t wparam, int64_t lparam) {
    switch(msg) {
    default:
        return DefWindowProcA(wnd, msg, wparam, lparam);
    }
}