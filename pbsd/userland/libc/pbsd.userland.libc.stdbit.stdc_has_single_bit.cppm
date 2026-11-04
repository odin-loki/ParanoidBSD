module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_has_single_bit;

export import pbsd.core;

/// stdc_has_single_bit from hbsd/src/lib/libc/stdbit/stdc_has_single_bit.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_has_single_bit_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
