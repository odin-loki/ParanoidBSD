module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.ulimit;

export import pbsd.core;

/// ulimit from hbsd/src/lib/libc/gen/ulimit.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long ulimit_cmd(int cmd, long limit) noexcept { (void)cmd; return limit; }

} // namespace pbsd::userland::libc
