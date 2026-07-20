module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getcap;

export import pbsd.core;

/// getcap from hbsd/src/lib/libc/gen/getcap.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getcap_file(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
