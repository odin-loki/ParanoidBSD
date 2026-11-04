module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sched_getaffinity;

export import pbsd.core;

/// sched_getaffinity from hbsd/src/lib/libc/gen/sched_getaffinity.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sched_getaffinity_pid(int pid) noexcept { return pid >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
