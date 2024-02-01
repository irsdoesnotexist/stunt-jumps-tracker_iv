#include <um/Windows.h>

#include "app.hpp"

#ifndef SJT4_WINDOWING_H
#define SJT4_WINDOWING_H
namespace sjt4_wndwng {

extern "C" typedef struct windowCreateInfo windowCreateInfo_t;
class Wnd_t;


#if defined (_WIN32)
extern "C" {
    typedef struct windowCreateInfo {
    } windowCreateInfo_t;
}

#define SJT4_MAIN_WND_CLS_NAME "sjt4_mainWindowCls"

#define SJT4_MAIN_WND_CREATE_INFO   windowCreateInfo_t  \
    {                                                   \
                                                        \
    };
//

#define SJT4_MAIN_WND_CLS_INFO      WNDCLASSA   \
    {                                           \
        .style       = CS_HREDRAW|CS_VREDRAW,   \
        .lpfnWndProc = nullptr,                 \
        .cbClsExtra = 0, .cbWndExtra = 0,       \
    };
//
class Wnd_t {
private:
    HWND hWnd;
public:
    bool init(const windowCreateInfo_t&) noexcept;
};

}
#endif

#endif