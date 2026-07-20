module;

export module pbsd.userland.libthr.setschedparam;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.sched;

/// pthread_setschedparam from hbsd/src/lib/libthr/thread/thr_setschedparam.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status setschedparam_thread(ThreadId tid, SchedPolicy policy,
                                                 const SchedParam& param) noexcept {
    return sched_setscheduler(tid, policy, param);
}

} // namespace pbsd::userland::libthr
