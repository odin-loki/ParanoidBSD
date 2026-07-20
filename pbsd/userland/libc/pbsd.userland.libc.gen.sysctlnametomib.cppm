module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sysctlnametomib;

export import pbsd.core;

/// sysctlnametomib from hbsd/src/lib/libc/gen/sysctlnametomib.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sysctlnametomib_name(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
