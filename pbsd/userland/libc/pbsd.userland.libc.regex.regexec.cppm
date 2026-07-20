module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.regex.regexec;

export import pbsd.core;

/// regexec from hbsd/src/lib/libc/regex/regexec.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status regexec_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
