module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.getgid;

export import pbsd.core;

/// getgid from hbsd/src/lib/libc/sys/getgid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned getgid_val() noexcept { return 0; }

} // namespace pbsd::userland::libc
