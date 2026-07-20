module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.gethostnamadr;

export import pbsd.core;

/// gethostnamadr from hbsd/src/lib/libc/net/gethostnamadr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gethostnamadr_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
