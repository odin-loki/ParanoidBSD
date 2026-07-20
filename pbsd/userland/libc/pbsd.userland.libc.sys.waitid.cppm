module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.waitid;

export import pbsd.core;

/// waitid from hbsd/src/lib/libc/sys/waitid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status waitid_idtype(int idtype) noexcept { (void)idtype; return Status::Ok; }

} // namespace pbsd::userland::libc
