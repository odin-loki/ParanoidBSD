module;

export module pbsd.userland.libthr.cond;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_cond concepts from hbsd/src/lib/libthr/thread/thr_cond.c
export namespace pbsd::userland::libthr {

struct Cond {
    unsigned waiters{0};
    bool signaled{false};
};

[[nodiscard]] inline Status cond_init(Cond& c) noexcept {
    c = Cond{};
    return Status::Ok;
}

[[nodiscard]] inline Status cond_wait(Cond& c, Mutex& m, ThreadId self) noexcept {
    if (m.owner != self) {
        return Status::Denied;
    }
    ++c.waiters;
    m.owner = kInvalidThread;
    m.lock_count = 0;
    if (c.signaled) {
        c.signaled = false;
        --c.waiters;
        return mutex_lock(m, self);
    }
    return Status::Ok;
}

[[nodiscard]] inline Status cond_signal(Cond& c) noexcept {
    if (c.waiters > 0) {
        c.signaled = true;
        --c.waiters;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status cond_broadcast(Cond& c) noexcept {
    c.signaled = c.waiters > 0;
    c.waiters = 0;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
