module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.getgroups;

export import pbsd.core;

/// getgroups from hbsd/src/lib/libc/sys/getgroups.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int getgroups_cnt(int size, unsigned* list) noexcept { (void)size; (void)list; return 0; }

} // namespace pbsd::userland::libc
