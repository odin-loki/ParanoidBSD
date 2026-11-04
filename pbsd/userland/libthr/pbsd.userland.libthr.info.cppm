module;

export module pbsd.userland.libthr.info;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_get_info from hbsd/src/lib/libthr/thread/thr_info.c
export namespace pbsd::userland::libthr {

struct ThreadInfo { ThreadId id{kInvalidThread}; bool running{false}; };

[[nodiscard]] inline ThreadInfo info_for(ThreadId tid) noexcept {
    ThreadInfo info{};
    info.id = tid;
    info.running = tid != kInvalidThread;
    return info;
}

} // namespace pbsd::userland::libthr
