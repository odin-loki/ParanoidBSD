module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.getrpcport;

export import pbsd.core;

/// getrpcport from hbsd/src/lib/libc/rpc/getrpcport.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getrpcport_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
