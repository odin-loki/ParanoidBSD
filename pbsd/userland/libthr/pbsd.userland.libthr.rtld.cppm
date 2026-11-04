module;

export module pbsd.userland.libthr.rtld;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_rtld from hbsd/src/lib/libthr/thread/thr_rtld.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status rtld_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
