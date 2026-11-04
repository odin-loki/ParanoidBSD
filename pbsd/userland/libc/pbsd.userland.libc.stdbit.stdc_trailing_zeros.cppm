module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_trailing_zeros;

export import pbsd.core;

/// stdc_trailing_zeros from hbsd/src/lib/libc/stdbit/stdc_trailing_zeros.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_trailing_zeros_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
