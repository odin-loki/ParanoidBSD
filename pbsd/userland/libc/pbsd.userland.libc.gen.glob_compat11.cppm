module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.glob_compat11;

export import pbsd.core;

/// glob_compat11 from hbsd/src/lib/libc/gen/glob_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status glob_compat11_pattern(const char* pat) noexcept { return pat == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
