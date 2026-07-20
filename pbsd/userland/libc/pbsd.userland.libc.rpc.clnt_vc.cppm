module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_vc;

export import pbsd.core;

/// clnt_vc from hbsd/src/lib/libc/rpc/clnt_vc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_vc_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
