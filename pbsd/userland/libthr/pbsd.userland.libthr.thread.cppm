module;

#include <cstdint>

export module pbsd.userland.libthr.thread;

import pbsd.core;
export import pbsd.userland.libthr.mutex;

/// pthread thread id concepts from hbsd/src/lib/libthr/thread/thr_create.c
export namespace pbsd::userland::libthr {

inline ThreadId g_main_thread{1};

struct ThreadAttr {
    unsigned stack_size{0};
    bool detached{false};
};

[[nodiscard]] inline ThreadId thread_self() noexcept { return g_main_thread; }

[[nodiscard]] inline Status thread_create(ThreadId& out, ThreadAttr attr,
                                            void (*start)(void*), void* arg) noexcept {
    (void)attr;
    (void)start;
    (void)arg;
    out = g_main_thread + 1;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
