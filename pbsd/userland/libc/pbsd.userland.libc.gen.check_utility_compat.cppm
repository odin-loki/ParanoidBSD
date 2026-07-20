module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.check_utility_compat;

export import pbsd.core;

/// check_utility_compat from hbsd/src/lib/libc/gen/check_utility_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool check_utility_compat_name(const char* name) noexcept { return name != nullptr; }

} // namespace pbsd::userland::libc
