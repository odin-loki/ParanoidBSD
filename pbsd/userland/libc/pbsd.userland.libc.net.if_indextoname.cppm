module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.if_indextoname;

export import pbsd.core;

/// if_indextoname from hbsd/src/lib/libc/net/if_indextoname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status if_indextoname_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
