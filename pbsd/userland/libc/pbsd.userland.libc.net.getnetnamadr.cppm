module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getnetnamadr;

export import pbsd.core;

/// getnetnamadr from hbsd/src/lib/libc/net/getnetnamadr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getnetnamadr_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
