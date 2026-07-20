module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.rpcb_clnt;

export import pbsd.core;

/// rpcb_clnt from hbsd/src/lib/libc/rpc/rpcb_clnt.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rpcb_clnt_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
