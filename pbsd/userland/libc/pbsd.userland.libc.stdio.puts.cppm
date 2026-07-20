module;

#include <cstddef>

export module pbsd.userland.libc.stdio.puts;

import pbsd.userland.libc.string;

/// puts from hbsd/src/lib/libc/stdio/puts.c (logic-only)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int puts(const char* s) noexcept {
    if (s == nullptr) {
        return -1;
    }
    return static_cast<int>(pbsd::userland::libc::strlen(s)) + 1;
}

} // namespace pbsd::userland::libc::stdio
