#include "app.hpp"
#include "windowing.hpp"

#define __windowCreateInfo_expand(wndInfo)      \
    wndInfo##.dwExStyle,                        \
    wndInfo##.lpClassName,                      \
    wndInfo##.lpWindowName,                     \
    wndInfo##.dwStyle,                          \
    wndInfo##.x,                                \
    wndInfo##.y,                                \
    wndInfo##.nWidth,                           \
    wndInfo##.nHeight,                          \
    wndInfo##.hParent,                          \
    wndInfo##.hMenu,                            \
    wndInfo##.hInstance,                        \
    wndInfo##.lpParam


// Not particuarly useful in this app but why not
std::shared_ptr<sjt4_wndwng::wndClass_t> sjt4_wndwng::wndClass_t::clsCheckNRegister (const WNDCLASS& ref_info, const HINSTANCE hInstance) {
    if( !sjt4_app::App_t::wndClasses.contains(ref_info.lpszClassName)) {    // Check if class' been registered
        
        auto p_newObj {std::make_shared<wndClass_t>(ref_info, hInstance)}; // If not - register (see constructor wndClass_t(WNDCLASS&))

        sjt4_app::App_t::wndClasses.insert (
            std::pair<LPCWSTR, std::weak_ptr<sjt4_wndwng::wndClass_t>>
            {ref_info.lpszClassName, p_newObj}    );

        return p_newObj;
    }
    else {  // Has been registered...
        auto it = sjt4_app::App_t::wndClasses.find(ref_info.lpszClassName);
        if(it->second.expired()) {  // But then unregistered
            return (it->second = std::make_shared<sjt4_wndwng::wndClass_t>(ref_info, hInstance)).lock();
        }
        else {                      // And still exists
            return it->second.lock();
        }
    }
}

sjt4_wndwng::wndClass_t::wndClass_t(const WNDCLASS& ref_info, const HINSTANCE hInstance) {
    this->name=ref_info.lpszClassName;
    this->app=hInstance;
    RegisterClass(&ref_info);
}

sjt4_wndwng::wndClass_t::~wndClass_t() {
    UnregisterClass(this->name, this->app);
}


sjt4_wndwng::Wnd_t::Wnd_t(const SJT4_wndInfo_t& in_ref_wndInfo, const WNDCLASS& in_ref_clsInfo) {
    this->p_cls=  wndClass_t::clsCheckNRegister(in_ref_clsInfo, sjt4_app::App_t::hinstance());
    this->handle= CreateWindowEx(__windowCreateInfo_expand(in_ref_wndInfo));
}