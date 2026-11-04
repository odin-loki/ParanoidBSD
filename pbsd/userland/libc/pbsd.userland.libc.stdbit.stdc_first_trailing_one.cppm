module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_first_trailing_one;

export import pbsd.core;

/// stdc_first_trailing_one from hbsd/src/lib/libc/stdbit/stdc_first_trailing_one.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_first_trailing_one_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
