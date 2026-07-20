module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_data;

export import pbsd.core;

/// res_data from hbsd/src/lib/libc/resolv/res_data.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_data_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
