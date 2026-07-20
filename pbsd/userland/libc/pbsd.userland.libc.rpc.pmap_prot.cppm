module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.pmap_prot;

export import pbsd.core;

/// pmap_prot from hbsd/src/lib/libc/rpc/pmap_prot.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pmap_prot_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
