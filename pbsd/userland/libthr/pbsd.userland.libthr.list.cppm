module;

export module pbsd.userland.libthr.list;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_list from hbsd/src/lib/libthr/thread/thr_list.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline bool list_contains(ThreadId tid) noexcept {
    return tid != kInvalidThread;
}

} // namespace pbsd::userland::libthr
