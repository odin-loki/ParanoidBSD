module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_query;

export import pbsd.core;

/// res_query from hbsd/src/lib/libc/resolv/res_query.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_query_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
