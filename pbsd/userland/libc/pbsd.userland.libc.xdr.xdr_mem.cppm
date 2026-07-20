module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_mem;

export import pbsd.core;

/// xdr_mem from hbsd/src/lib/libc/xdr/xdr_mem.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_mem_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
