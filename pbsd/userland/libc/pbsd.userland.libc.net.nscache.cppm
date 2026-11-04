module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.nscache;

export import pbsd.core;

/// nscache from hbsd/src/lib/libc/net/nscache.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nscache_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
