module;

export module pbsd.userland.libthr.getschedparam;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.sched;

/// pthread_getschedparam from hbsd/src/lib/libthr/thread/thr_getschedparam.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status getschedparam_thread(ThreadId tid, SchedPolicy& policy,
                                                 SchedParam& param) noexcept {
    const Status sp = sched_getparam(tid, param);
    if (sp != Status::Ok) {
        return sp;
    }
    return sched_getscheduler(tid, policy);
}

} // namespace pbsd::userland::libthr
