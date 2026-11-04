module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fts;

export import pbsd.core;

/// fts from hbsd/src/lib/libc/gen/fts.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fts_open_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
