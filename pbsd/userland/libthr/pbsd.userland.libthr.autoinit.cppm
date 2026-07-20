module;

export module pbsd.userland.libthr.autoinit;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_autoinit from hbsd/src/lib/libthr/thread/thr_autoinit.c
export namespace pbsd::userland::libthr {

inline bool g_thr_initialized{false};

inline void autoinit_once() noexcept { g_thr_initialized = true; }

} // namespace pbsd::userland::libthr
