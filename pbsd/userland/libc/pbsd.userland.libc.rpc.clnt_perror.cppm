module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.clnt_perror;

export import pbsd.core;

/// clnt_perror from hbsd/src/lib/libc/rpc/clnt_perror.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clnt_perror_init() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
