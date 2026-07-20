module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_generic;

export import pbsd.core;

/// clnt_generic from hbsd/src/lib/libc/rpc/clnt_generic.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_generic_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
