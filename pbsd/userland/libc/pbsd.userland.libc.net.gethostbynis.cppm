module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.gethostbynis;

export import pbsd.core;

/// gethostbynis from hbsd/src/lib/libc/net/gethostbynis.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gethostbynis_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
