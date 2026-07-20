module;

export module pbsd.userland.libthr.concurrency;

import pbsd.core;

/// pthread_get/setconcurrency from hbsd/src/lib/libthr/thread/thr_concurrency.c
export namespace pbsd::userland::libthr {

inline int g_current_concurrency = 0;

[[nodiscard]] inline int getconcurrency() noexcept { return g_current_concurrency; }

[[nodiscard]] inline Status setconcurrency(int new_level) noexcept {
    if (new_level < 0) {
        return Status::Invalid;
    }
    g_current_concurrency = new_level;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
