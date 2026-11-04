module;

export module pbsd.userland.libthr.testcancel;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_testcancel from hbsd/src/lib/libthr/thread/thr_cancel.c
export namespace pbsd::userland::libthr {

inline void testcancel_point(ThreadId self) noexcept {
    (void)self;
}

} // namespace pbsd::userland::libthr
