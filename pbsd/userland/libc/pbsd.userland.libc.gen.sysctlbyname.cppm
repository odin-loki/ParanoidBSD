module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sysctlbyname;

export import pbsd.core;

/// sysctlbyname from hbsd/src/lib/libc/gen/sysctlbyname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sysctlbyname_name(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
