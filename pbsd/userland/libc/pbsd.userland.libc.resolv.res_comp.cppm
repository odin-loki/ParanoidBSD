module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_comp;

export import pbsd.core;

/// res_comp from hbsd/src/lib/libc/resolv/res_comp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_comp_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
