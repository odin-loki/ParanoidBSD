module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.arc4random_compat;

export import pbsd.core;

/// arc4random_compat from hbsd/src/lib/libc/gen/arc4random_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned arc4random_compat_val() noexcept { return 0u; }

} // namespace pbsd::userland::libc
