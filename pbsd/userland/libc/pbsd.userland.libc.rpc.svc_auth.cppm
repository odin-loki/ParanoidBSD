module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_auth;

export import pbsd.core;

/// svc_auth from hbsd/src/lib/libc/rpc/svc_auth.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_auth_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
