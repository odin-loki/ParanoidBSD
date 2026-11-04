module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.inotify;

export import pbsd.core;

/// inotify from hbsd/src/lib/libc/gen/inotify.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inotify_init_flags(int flags) noexcept { (void)flags; return Status::Ok; }

} // namespace pbsd::userland::libc
