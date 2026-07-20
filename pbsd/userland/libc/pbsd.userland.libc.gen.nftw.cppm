module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.nftw;

export import pbsd.core;

/// nftw from hbsd/src/lib/libc/gen/nftw.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nftw_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
