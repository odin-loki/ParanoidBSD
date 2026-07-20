module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.getegid;

export import pbsd.core;

/// getegid from hbsd/src/lib/libc/sys/getegid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned getegid_val() noexcept { return 0; }

} // namespace pbsd::userland::libc
