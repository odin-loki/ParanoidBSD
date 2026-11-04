module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_bit_width;

export import pbsd.core;

/// stdc_bit_width from hbsd/src/lib/libc/stdbit/stdc_bit_width.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_bit_width_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
