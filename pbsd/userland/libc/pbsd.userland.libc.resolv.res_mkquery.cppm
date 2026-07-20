module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_mkquery;

export import pbsd.core;

/// res_mkquery from hbsd/src/lib/libc/resolv/res_mkquery.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_mkquery_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
