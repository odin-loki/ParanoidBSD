module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_bit_floor;

export import pbsd.core;

/// stdc_bit_floor from hbsd/src/lib/libc/stdbit/stdc_bit_floor.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_bit_floor_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
