module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc;

export import pbsd.core;

/// svc from hbsd/src/lib/libc/rpc/svc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
