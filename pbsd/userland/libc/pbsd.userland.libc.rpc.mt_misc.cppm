module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.mt_misc;

export import pbsd.core;

/// mt_misc from hbsd/src/lib/libc/rpc/mt_misc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mt_misc_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
