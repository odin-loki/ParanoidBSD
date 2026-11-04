module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_findzonecut;

export import pbsd.core;

/// res_findzonecut from hbsd/src/lib/libc/resolv/res_findzonecut.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_findzonecut_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
