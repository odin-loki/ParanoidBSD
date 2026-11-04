module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_sizeof;

export import pbsd.core;

/// xdr_sizeof from hbsd/src/lib/libc/xdr/xdr_sizeof.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_sizeof_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
