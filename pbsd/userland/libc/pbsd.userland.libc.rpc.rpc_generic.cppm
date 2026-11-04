module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.rpc_generic;

export import pbsd.core;

/// rpc_generic from hbsd/src/lib/libc/rpc/rpc_generic.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rpc_generic_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
