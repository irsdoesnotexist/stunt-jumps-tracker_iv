#include <um/Windows.h>

#include <cstdint>

#include "app.hpp"
#include "windowing.hpp"


std::int32_t __declspec(stdcall) WinMain(HINSTANCE hInstance, HINSTANCE [[maybe_unused]], char* cmdln, int32_t cmdshow) {
    sjt4_app::App_t::instance.init();

    return EXIT_SUCCESS;
}