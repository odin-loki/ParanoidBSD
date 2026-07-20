module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_stdio;

export import pbsd.core;

/// xdr_stdio from hbsd/src/lib/libc/xdr/xdr_stdio.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_stdio_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
