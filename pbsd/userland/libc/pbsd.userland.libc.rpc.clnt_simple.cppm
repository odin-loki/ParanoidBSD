module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_simple;

export import pbsd.core;

/// clnt_simple from hbsd/src/lib/libc/rpc/clnt_simple.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_simple_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
