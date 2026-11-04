module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_auth_unix;

export import pbsd.core;

/// svc_auth_unix from hbsd/src/lib/libc/rpc/svc_auth_unix.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_auth_unix_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
