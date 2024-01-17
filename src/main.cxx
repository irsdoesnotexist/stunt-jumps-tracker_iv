#include <cstdint>
#include <cstdio>

#include <vulkan\vulkan.h>

#if defined(_WIN64)
#include <um\Windows.h>

std::int32_t __attribute((stdcall)) WinMain(HINSTANCE__* progInstance, HINSTANCE__*, char* cmdline, std::int32_t cmdShow) {
    return EXIT_SUCCESS;
}
#endif