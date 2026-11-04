module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.tfind;

export import pbsd.core;

/// tfind from hbsd/src/lib/libc/stdlib/tfind.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* tfind_key(const void* key) noexcept { (void)key; return nullptr; }

} // namespace pbsd::userland::libc
