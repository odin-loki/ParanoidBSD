module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_state;

export import pbsd.core;

/// res_state from hbsd/src/lib/libc/resolv/res_state.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_state_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
