module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.rpcb_st_xdr;

export import pbsd.core;

/// rpcb_st_xdr from hbsd/src/lib/libc/rpc/rpcb_st_xdr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rpcb_st_xdr_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
