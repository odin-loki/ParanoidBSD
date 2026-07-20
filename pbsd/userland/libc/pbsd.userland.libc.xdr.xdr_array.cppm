module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_array;

export import pbsd.core;

/// xdr_array from hbsd/src/lib/libc/xdr/xdr_array.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_array_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
