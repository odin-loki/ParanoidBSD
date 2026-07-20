module;
#include <cstddef>

export module pbsd.userland.libc.stdlib.realpath;

export import pbsd.core;

/// realpath from hbsd/src/lib/libc/stdlib/realpath.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status realpath_validate(const char* path, char* resolved,
                                              std::size_t len) noexcept {
    if (path == nullptr || resolved == nullptr || len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
