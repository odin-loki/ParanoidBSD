module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.getrpcent;

export import pbsd.core;

/// getrpcent from hbsd/src/lib/libc/rpc/getrpcent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getrpcent_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
