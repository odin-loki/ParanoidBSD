module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_float;

export import pbsd.core;

/// xdr_float from hbsd/src/lib/libc/xdr/xdr_float.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_float_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
