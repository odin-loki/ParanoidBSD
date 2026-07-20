module;

export module pbsd.userland.libthr.malloc;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_malloc from hbsd/src/lib/libthr/thread/thr_malloc.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status malloc_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
