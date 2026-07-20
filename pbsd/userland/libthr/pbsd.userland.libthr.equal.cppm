module;

export module pbsd.userland.libthr.equal;

import pbsd.userland.libthr.mutex;

/// pthread_equal from hbsd/src/lib/libthr/thread/thr_equal.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline bool thread_equal(ThreadId a, ThreadId b) noexcept { return a == b; }

} // namespace pbsd::userland::libthr
