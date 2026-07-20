module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.setgroups;

export import pbsd.core;

/// setgroups from hbsd/src/lib/libc/sys/setgroups.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setgroups_cnt(int size, const unsigned* list) noexcept { (void)size; (void)list; return Status::Ok; }

} // namespace pbsd::userland::libc
