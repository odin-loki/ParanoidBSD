module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_bit_ceil;

export import pbsd.core;

/// stdc_bit_ceil from hbsd/src/lib/libc/stdbit/stdc_bit_ceil.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_bit_ceil_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
