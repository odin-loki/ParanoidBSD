module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.rcmdsh;

export import pbsd.core;

/// rcmdsh from hbsd/src/lib/libc/net/rcmdsh.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rcmdsh_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
