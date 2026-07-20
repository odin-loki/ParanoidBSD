module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.bindresvport;

export import pbsd.core;

/// bindresvport from hbsd/src/lib/libc/rpc/bindresvport.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status bindresvport_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
