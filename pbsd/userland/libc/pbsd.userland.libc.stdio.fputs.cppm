module;

#include <cstddef>

export module pbsd.userland.libc.stdio.fputs;

import pbsd.userland.libc.stdio.file;
import pbsd.userland.libc.string;

/// fputs from hbsd/src/lib/libc/stdio/fputs.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fputs(const char* s, IoFile* fp) noexcept {
    if (s == nullptr || fp == nullptr) {
        return -1;
    }
    return static_cast<int>(pbsd::userland::libc::strlen(s));
}

} // namespace pbsd::userland::libc::stdio
