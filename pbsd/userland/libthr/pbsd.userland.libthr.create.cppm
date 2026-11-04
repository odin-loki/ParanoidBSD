module;

export module pbsd.userland.libthr.create;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_create from hbsd/src/lib/libthr/thread/thr_create.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status pthread_create_thread(ThreadId& out, ThreadAttr attr,
                                                  void (*start)(void*), void* arg) noexcept {
    return thread_create(out, attr, start, arg);
}

} // namespace pbsd::userland::libthr
