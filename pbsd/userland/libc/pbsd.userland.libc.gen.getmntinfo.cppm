module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getmntinfo;

export import pbsd.core;

/// getmntinfo from hbsd/src/lib/libc/gen/getmntinfo.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int getmntinfo_count(int flags) noexcept { (void)flags; return 0; }

} // namespace pbsd::userland::libc
