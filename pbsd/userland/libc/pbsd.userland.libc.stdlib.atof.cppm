module;

#include <cctype>

export module pbsd.userland.libc.stdlib.atof;

import pbsd.userland.libc.stdlib.strtod;

/// atof from hbsd/src/lib/libc/stdlib/atof.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double atof(const char* str) noexcept {
    return strtod(str, nullptr);
}

} // namespace pbsd::userland::libc
