module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strfmon;

export import pbsd.core;

/// strfmon from hbsd/src/lib/libc/stdlib/strfmon.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strfmon_buf(char* s, std::size_t max, const char* fmt) noexcept {
    if (s == nullptr || max == 0 || fmt == nullptr) return Status::Invalid;
    s[0] = '\0';
    return Status::Ok;
}

} // namespace pbsd::userland::libc
