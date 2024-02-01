#include <cstdint>

#include <vulkan/vulkan.h>
// vulkan-1.lib => vulkan-1.dll

#include "app.hpp"

#if defined(_WIN32)
#include <um/Windows.h>
// ntdll.dll, User32.dll, kernel32.dll
//__declspec(dllimport) std::int32_t __stdcall NtTerminateProcess(void*, std::int32_t exitcode);
//#define EXIT_PROC(CODE) NtTerminateProcess(nullptr, CODE)

#define RETURN_SUCCESS EXIT_SUCCESS

std::int32_t WinMain(HINSTANCE hInstance, HINSTANCE [[maybe_unused]], char* cmdln, std::int32_t show) {
#endif
    sjt4_app::App_t::instance.init();

    return RETURN_SUCCESS;
}