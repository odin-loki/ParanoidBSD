module;

export module pbsd.userland.libthr.syscalls;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_syscalls from hbsd/src/lib/libthr/thread/thr_syscalls.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status syscalls_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
