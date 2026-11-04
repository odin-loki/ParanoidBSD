module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.pmap_getport;

export import pbsd.core;

/// pmap_getport from hbsd/src/lib/libc/rpc/pmap_getport.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pmap_getport_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
