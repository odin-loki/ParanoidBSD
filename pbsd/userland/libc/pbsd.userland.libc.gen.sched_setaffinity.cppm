module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sched_setaffinity;

export import pbsd.core;

/// sched_setaffinity from hbsd/src/lib/libc/gen/sched_setaffinity.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sched_setaffinity_pid(int pid) noexcept { return pid >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
