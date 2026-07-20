module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdbit.stdc_leading_ones;

export import pbsd.core;

/// stdc_leading_ones from hbsd/src/lib/libc/stdbit/stdc_leading_ones.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stdc_leading_ones_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
