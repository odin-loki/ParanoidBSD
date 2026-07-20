module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.rpcsec_gss_stub;

export import pbsd.core;

/// rpcsec_gss_stub from hbsd/src/lib/libc/rpc/rpcsec_gss_stub.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rpcsec_gss_stub_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
