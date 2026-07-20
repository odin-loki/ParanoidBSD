module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_first_leading_zero;

export import pbsd.core;

/// stdc_first_leading_zero from hbsd/src/lib/libc/stdbit/stdc_first_leading_zero.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_first_leading_zero_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
