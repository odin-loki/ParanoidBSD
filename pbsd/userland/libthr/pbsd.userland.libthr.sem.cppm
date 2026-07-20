module;

export module pbsd.userland.libthr.sem;

import pbsd.core;

/// sem concepts from hbsd/src/lib/libthr/thread/thr_sem.c
export namespace pbsd::userland::libthr {

struct Semaphore {
    unsigned value{0};
};

[[nodiscard]] inline Status sem_init(Semaphore& s, unsigned initial) noexcept {
    s.value = initial;
    return Status::Ok;
}

[[nodiscard]] inline Status sem_post(Semaphore& s) noexcept {
    ++s.value;
    return Status::Ok;
}

[[nodiscard]] inline Status sem_wait(Semaphore& s) noexcept {
    if (s.value == 0) {
        return Status::Busy;
    }
    --s.value;
    return Status::Ok;
}

[[nodiscard]] inline Status sem_trywait(Semaphore& s) noexcept { return sem_wait(s); }

} // namespace pbsd::userland::libthr
