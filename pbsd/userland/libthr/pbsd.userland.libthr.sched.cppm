module;

#include <cstdint>

export module pbsd.userland.libthr.sched;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread scheduling from hbsd/src/lib/libthr/thread/thr_create.c (policy attrs)
export namespace pbsd::userland::libthr {

enum class SchedPolicy : std::uint8_t { Other, Fifo, RoundRobin };

struct SchedParam {
    int priority{0};
};

[[nodiscard]] inline Status sched_getparam(ThreadId tid, SchedParam& param) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    param = SchedParam{};
    return Status::Ok;
}

[[nodiscard]] inline Status sched_setparam(ThreadId tid, const SchedParam& param) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)param;
    return Status::Ok;
}

[[nodiscard]] inline Status sched_getscheduler(ThreadId tid, SchedPolicy& policy) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    policy = SchedPolicy::Other;
    return Status::Ok;
}

[[nodiscard]] inline Status sched_setscheduler(ThreadId tid, SchedPolicy policy,
                                               const SchedParam& param) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)policy;
    (void)param;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
