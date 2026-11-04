module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.setgid;

export import pbsd.core;

/// setgid from hbsd/src/lib/libc/sys/setgid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setgid_id(unsigned gid) noexcept { (void)gid; return Status::Ok; }

} // namespace pbsd::userland::libc
