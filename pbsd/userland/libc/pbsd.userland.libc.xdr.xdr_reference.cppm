module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_reference;

export import pbsd.core;

/// xdr_reference from hbsd/src/lib/libc/xdr/xdr_reference.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_reference_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
