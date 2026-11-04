module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.des_soft;

export import pbsd.core;

/// des_soft from hbsd/src/lib/libc/rpc/des_soft.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status des_soft_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
