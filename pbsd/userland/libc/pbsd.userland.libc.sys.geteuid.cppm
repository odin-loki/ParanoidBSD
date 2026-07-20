module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.geteuid;

export import pbsd.core;

/// geteuid from hbsd/src/lib/libc/sys/geteuid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned geteuid_val() noexcept { return 0; }

} // namespace pbsd::userland::libc
