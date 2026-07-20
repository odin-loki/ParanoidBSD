module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_bcast;

export import pbsd.core;

/// clnt_bcast from hbsd/src/lib/libc/rpc/clnt_bcast.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_bcast_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
