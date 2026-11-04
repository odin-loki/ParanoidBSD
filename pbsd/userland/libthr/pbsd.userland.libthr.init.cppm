module;

export module pbsd.userland.libthr.init;

import pbsd.core;

/// pthread library init from hbsd/src/lib/libthr/thread/thr_init.c
export namespace pbsd::userland::libthr {

inline bool g_thr_initialized{false};

[[nodiscard]] inline Status thread_library_init() noexcept {
    g_thr_initialized = true;
    return Status::Ok;
}

[[nodiscard]] inline bool thread_library_ready() noexcept { return g_thr_initialized; }

} // namespace pbsd::userland::libthr
