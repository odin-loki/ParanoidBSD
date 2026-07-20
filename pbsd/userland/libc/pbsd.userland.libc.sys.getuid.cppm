module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.getuid;

export import pbsd.core;

/// getuid from hbsd/src/lib/libc/sys/getuid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned getuid_val() noexcept { return 0; }

} // namespace pbsd::userland::libc
