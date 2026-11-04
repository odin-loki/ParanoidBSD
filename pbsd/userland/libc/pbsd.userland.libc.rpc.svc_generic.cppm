module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_generic;

export import pbsd.core;

/// svc_generic from hbsd/src/lib/libc/rpc/svc_generic.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_generic_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
