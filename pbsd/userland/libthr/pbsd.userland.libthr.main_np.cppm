module;

export module pbsd.userland.libthr.main_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_main_np from hbsd/src/lib/libthr/thread/thr_main_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline bool main_np_is_main(ThreadId tid) noexcept {
    return tid == thread_self();
}

} // namespace pbsd::userland::libthr
