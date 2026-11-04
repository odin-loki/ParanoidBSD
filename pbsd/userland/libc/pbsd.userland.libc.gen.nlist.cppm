module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.nlist;

export import pbsd.core;

/// nlist from hbsd/src/lib/libc/gen/nlist.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nlist_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
