module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.xdr.xdr_rec;

export import pbsd.core;

/// xdr_rec from hbsd/src/lib/libc/xdr/xdr_rec.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdr_rec_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
