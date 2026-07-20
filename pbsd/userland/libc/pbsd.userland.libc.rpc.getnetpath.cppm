module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.getnetpath;

export import pbsd.core;

/// getnetpath from hbsd/src/lib/libc/rpc/getnetpath.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getnetpath_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
