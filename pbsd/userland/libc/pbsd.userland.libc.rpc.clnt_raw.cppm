module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_raw;

export import pbsd.core;

/// clnt_raw from hbsd/src/lib/libc/rpc/clnt_raw.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_raw_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
