module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pw_scan;

export import pbsd.core;

/// pw_scan from hbsd/src/lib/libc/gen/pw_scan.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pw_scan_line(const char* line) noexcept {
    if (line == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
