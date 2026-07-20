module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.setuid;

export import pbsd.core;

/// setuid from hbsd/src/lib/libc/sys/setuid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setuid_id(unsigned uid) noexcept { (void)uid; return Status::Ok; }

} // namespace pbsd::userland::libc
