module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr;

export import pbsd.core;

/// xdr from hbsd/src/lib/libc/xdr/xdr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
