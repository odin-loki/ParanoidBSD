module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.rpcdname;

export import pbsd.core;

/// rpcdname from hbsd/src/lib/libc/rpc/rpcdname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rpcdname_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
