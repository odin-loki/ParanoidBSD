module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.regex.regfree;

export import pbsd.core;

/// regfree from hbsd/src/lib/libc/regex/regfree.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status regfree_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
