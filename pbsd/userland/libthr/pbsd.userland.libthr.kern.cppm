module;

export module pbsd.userland.libthr.kern;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_kern from hbsd/src/lib/libthr/thread/thr_kern.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status kern_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
