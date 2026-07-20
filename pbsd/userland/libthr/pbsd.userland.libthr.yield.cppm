module;

export module pbsd.userland.libthr.yield;

import pbsd.core;

/// pthread_yield/sched_yield from hbsd/src/lib/libthr/thread/thr_yield.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_yield() noexcept { return Status::Ok; }

[[nodiscard]] inline Status sched_yield() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
