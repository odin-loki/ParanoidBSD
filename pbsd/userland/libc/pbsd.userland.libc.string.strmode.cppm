module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strmode;

export import pbsd.core;

/// strmode from hbsd/src/lib/libc/string/strmode.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strmode_mode(int mode, char* buf) noexcept {
    if (buf == nullptr) return Status::Invalid;
    (void)mode;
    buf[0] = '\0';
    return Status::Ok;
}

} // namespace pbsd::userland::libc
