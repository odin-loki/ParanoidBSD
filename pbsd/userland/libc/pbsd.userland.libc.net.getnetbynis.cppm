module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getnetbynis;

export import pbsd.core;

/// getnetbynis from hbsd/src/lib/libc/net/getnetbynis.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getnetbynis_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
