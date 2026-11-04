module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.key_prot_xdr;

export import pbsd.core;

/// key_prot_xdr from hbsd/src/lib/libc/rpc/key_prot_xdr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status key_prot_xdr_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
