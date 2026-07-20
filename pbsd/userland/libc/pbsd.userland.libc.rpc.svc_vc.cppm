module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_vc;

export import pbsd.core;

/// svc_vc from hbsd/src/lib/libc/rpc/svc_vc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_vc_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
