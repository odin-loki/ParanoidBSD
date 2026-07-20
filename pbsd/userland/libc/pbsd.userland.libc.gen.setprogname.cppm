module;

#include <cstddef>

export module pbsd.userland.libc.gen.setprogname;

import pbsd.userland.libc.gen.getprogname;

/// setprogname from hbsd/src/lib/libc/gen/setprogname.c
export namespace pbsd::userland::libc {

inline void setprogname(const char* name) noexcept {
    const char* src = (name != nullptr && name[0] != '\0') ? name : "pbsd";
    std::size_t i = 0;
    while (src[i] != '\0' && i + 1 < sizeof(g_progname_buf)) {
        g_progname_buf[i] = src[i];
        ++i;
    }
    g_progname_buf[i] = '\0';
}

} // namespace pbsd::userland::libc
