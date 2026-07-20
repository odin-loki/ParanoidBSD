module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.netnamer;

export import pbsd.core;

/// netnamer from hbsd/src/lib/libc/rpc/netnamer.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status netnamer_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
