// Wave 0: freestanding C++23 TU — exports extern "C" entry points; calls C glue.
#include "pbsd_cxx_hello.h"

namespace {

struct HelloState {
    int ready{0};
    int c_tag_ok{0};
};

HelloState g_state{};

} // namespace

extern "C" int pbsd_cxx_hello_init(void) {
    const char* tag = pbsd_cxx_hello_c_tag();
    g_state.c_tag_ok = (tag != nullptr && tag[0] == 'p') ? 1 : 0;
    g_state.ready = g_state.c_tag_ok;
    return g_state.ready ? 0 : 1;
}

extern "C" int pbsd_cxx_hello_fini(void) {
    g_state.ready = 0;
    g_state.c_tag_ok = 0;
    return 0;
}

extern "C" const char* pbsd_cxx_hello_version(void) {
    return "pbsd_cxx_hello/0.1 C++23";
}
