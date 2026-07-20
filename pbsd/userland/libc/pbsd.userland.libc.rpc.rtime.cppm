module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.rtime;

export import pbsd.core;

/// rtime from hbsd/src/lib/libc/rpc/rtime.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rtime_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
