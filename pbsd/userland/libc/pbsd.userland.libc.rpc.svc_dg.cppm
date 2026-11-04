module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_dg;

export import pbsd.core;

/// svc_dg from hbsd/src/lib/libc/rpc/svc_dg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_dg_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
