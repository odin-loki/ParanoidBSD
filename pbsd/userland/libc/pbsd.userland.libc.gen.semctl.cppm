module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.semctl;

export import pbsd.core;

/// semctl from hbsd/src/lib/libc/gen/semctl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status semctl_cmd(int semid, int semnum, int cmd) noexcept { (void)semid; (void)semnum; (void)cmd; return Status::Ok; }

} // namespace pbsd::userland::libc
