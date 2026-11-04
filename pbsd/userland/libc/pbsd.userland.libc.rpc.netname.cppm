module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.netname;

export import pbsd.core;

/// netname from hbsd/src/lib/libc/rpc/netname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status netname_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
