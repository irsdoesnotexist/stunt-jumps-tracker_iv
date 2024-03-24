#include "app.hpp"


bool sjt4::app::init(const HINSTANCE hInst) noexcept{
    const WNDCLASSA mainWndClsInfo{
        .style = CS_HREDRAW|CS_VREDRAW,
        .lpfnWndProc = sjt4::mainWndProc,
        .cbClsExtra = NULL, .cbWndExtra = NULL,
        .hInstance = hInst, .hIcon = nullptr,
        .hCursor = nullptr, .hbrBackground = nullptr,
        .lpszMenuName = nullptr,
        .lpszClassName = sjt4::mainWndClsName
    };
    RegisterClass(&mainWndClsInfo);
    return (sjt4::app::mainWindow = CreateWindow(
        sjt4::mainWndClsName,
        sjt4::appName,
        WS_CAPTION|WS_MINIMIZEBOX|WS_SIZEBOX|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInst,
        nullptr
    )) != nullptr;
}