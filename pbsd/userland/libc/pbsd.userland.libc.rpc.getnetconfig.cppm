module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.getnetconfig;

export import pbsd.core;

/// getnetconfig from hbsd/src/lib/libc/rpc/getnetconfig.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getnetconfig_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
