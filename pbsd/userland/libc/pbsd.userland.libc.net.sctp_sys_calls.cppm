module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.sctp_sys_calls;

export import pbsd.core;

/// sctp_sys_calls from hbsd/src/lib/libc/net/sctp_sys_calls.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sctp_sys_calls_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
