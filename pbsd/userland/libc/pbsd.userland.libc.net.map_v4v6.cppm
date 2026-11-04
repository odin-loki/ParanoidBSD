module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.map_v4v6;

export import pbsd.core;

/// map_v4v6 from hbsd/src/lib/libc/net/map_v4v6.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status map_v4v6_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
