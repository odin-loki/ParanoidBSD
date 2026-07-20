module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.rcmd;

export import pbsd.core;

/// rcmd from hbsd/src/lib/libc/net/rcmd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rcmd_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
