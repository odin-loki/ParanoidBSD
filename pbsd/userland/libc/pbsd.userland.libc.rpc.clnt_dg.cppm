module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_dg;

export import pbsd.core;

/// clnt_dg from hbsd/src/lib/libc/rpc/clnt_dg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_dg_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
