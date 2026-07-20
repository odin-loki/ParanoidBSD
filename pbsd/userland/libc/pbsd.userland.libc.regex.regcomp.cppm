module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.regex.regcomp;

export import pbsd.core;

/// regcomp from hbsd/src/lib/libc/regex/regcomp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status regcomp_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
