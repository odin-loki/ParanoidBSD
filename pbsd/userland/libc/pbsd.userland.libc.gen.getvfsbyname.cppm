module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getvfsbyname;

export import pbsd.core;

/// getvfsbyname from hbsd/src/lib/libc/gen/getvfsbyname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getvfsbyname_name(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
