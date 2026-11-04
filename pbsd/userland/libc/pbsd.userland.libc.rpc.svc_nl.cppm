module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_nl;

export import pbsd.core;

/// svc_nl from hbsd/src/lib/libc/rpc/svc_nl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_nl_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
