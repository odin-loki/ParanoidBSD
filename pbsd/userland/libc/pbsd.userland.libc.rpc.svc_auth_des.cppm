module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_auth_des;

export import pbsd.core;

/// svc_auth_des from hbsd/src/lib/libc/rpc/svc_auth_des.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_auth_des_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
