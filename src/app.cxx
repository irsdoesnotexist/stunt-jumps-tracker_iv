#include <memory>
#include <unordered_map>

#include "app.hpp"

#include <windowing.hpp>

bool sjt4_app::App_t::init() {
    App_t::appInst = GetModuleHandleA(NULL);

}

HINSTANCE sjt4_app::App_t::instance() {
    return sjt4_app::App_t::instance;
}