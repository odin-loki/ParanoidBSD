module;
#include <cstdint>

export module pbsd.userland.libthr.getthreadid_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_getthreadid_np from hbsd/src/lib/libthr/thread/thr_getthreadid_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline std::uint64_t getthreadid_np(ThreadId tid) noexcept {
    return static_cast<std::uint64_t>(tid);
}

} // namespace pbsd::userland::libthr
