module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_raw;

export import pbsd.core;

/// svc_raw from hbsd/src/lib/libc/rpc/svc_raw.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_raw_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
