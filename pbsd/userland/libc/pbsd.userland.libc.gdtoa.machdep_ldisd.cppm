module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.machdep_ldisd;

export import pbsd.core;

/// machdep_ldisd from hbsd/src/lib/libc/gdtoa/machdep_ldisd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status machdep_ldisd_buf(char* buf, std::size_t len) noexcept {
    if (buf == nullptr || len == 0) return Status::Invalid;
    buf[0] = '\0';
    return Status::Ok;
}

} // namespace pbsd::userland::libc
