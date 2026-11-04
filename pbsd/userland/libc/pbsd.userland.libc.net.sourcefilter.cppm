module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.sourcefilter;

export import pbsd.core;

/// sourcefilter from hbsd/src/lib/libc/net/sourcefilter.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sourcefilter_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
