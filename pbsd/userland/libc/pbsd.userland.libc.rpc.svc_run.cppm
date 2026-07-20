module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.svc_run;

export import pbsd.core;

/// svc_run from hbsd/src/lib/libc/rpc/svc_run.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status svc_run_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
