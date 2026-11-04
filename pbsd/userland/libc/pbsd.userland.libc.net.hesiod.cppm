module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.hesiod;

export import pbsd.core;

/// hesiod from hbsd/src/lib/libc/net/hesiod.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status hesiod_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
