module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getproto;

export import pbsd.core;

/// getproto from hbsd/src/lib/libc/net/getproto.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getproto_name(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
