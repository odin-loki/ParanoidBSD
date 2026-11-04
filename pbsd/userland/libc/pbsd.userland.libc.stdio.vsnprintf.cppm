module;

#include <cstdarg>
#include <cstddef>

export module pbsd.userland.libc.stdio.vsnprintf;

import pbsd.userland.libc.stdio.sprintf;

/// vsnprintf from hbsd/src/lib/libc/stdio/vsnprintf.c (va_list passthrough stub)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int vsnprintf(char* str, std::size_t size, const char* fmt,
                                   va_list ap) noexcept {
    (void)ap;
    return snprintf(str, size, fmt);
}

} // namespace pbsd::userland::libc::stdio
