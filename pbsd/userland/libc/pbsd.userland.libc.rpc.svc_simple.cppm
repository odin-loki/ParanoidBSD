module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_simple;

export import pbsd.core;

/// svc_simple from hbsd/src/lib/libc/rpc/svc_simple.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_simple_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
