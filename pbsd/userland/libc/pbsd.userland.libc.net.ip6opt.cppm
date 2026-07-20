module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.ip6opt;

export import pbsd.core;

/// ip6opt from hbsd/src/lib/libc/net/ip6opt.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ip6opt_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
