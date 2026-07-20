module;

export module pbsd.userland.libthr.cleanup;

import pbsd.core;

/// pthread_cleanup_push/pop from hbsd/src/lib/libthr/thread/thr_clean.c
export namespace pbsd::userland::libthr {

inline constexpr int kMaxCleanupDepth = 8;

struct CleanupHandler {
    void (*routine)(void*){nullptr};
    void* arg{nullptr};
};

inline CleanupHandler g_cleanup_stack[kMaxCleanupDepth]{};
inline int g_cleanup_depth = 0;

inline void cleanup_push(void (*routine)(void*), void* arg) noexcept {
    if (g_cleanup_depth >= kMaxCleanupDepth) {
        return;
    }
    g_cleanup_stack[g_cleanup_depth].routine = routine;
    g_cleanup_stack[g_cleanup_depth].arg = arg;
    ++g_cleanup_depth;
}

inline void cleanup_pop(int execute) noexcept {
    if (g_cleanup_depth <= 0) {
        return;
    }
    --g_cleanup_depth;
    CleanupHandler& old = g_cleanup_stack[g_cleanup_depth];
    if (execute && old.routine != nullptr) {
        old.routine(old.arg);
    }
    old.routine = nullptr;
    old.arg = nullptr;
}

} // namespace pbsd::userland::libthr
