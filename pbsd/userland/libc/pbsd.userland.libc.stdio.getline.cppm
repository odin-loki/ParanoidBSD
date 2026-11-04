module;
#include <cstddef>

export module pbsd.userland.libc.stdio.getline;

export import pbsd.core;

/// getline from hbsd/src/lib/libc/stdio/getline.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> getline_need(std::size_t len, char delim) noexcept {
    if (delim == '\0') {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(len + 1);
}

} // namespace pbsd::userland::libc
