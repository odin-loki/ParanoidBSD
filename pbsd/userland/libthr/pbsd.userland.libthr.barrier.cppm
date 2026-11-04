module;

export module pbsd.userland.libthr.barrier;

import pbsd.core;

/// pthread_barrier from hbsd/src/lib/libthr/thread/thr_barrier.c
export namespace pbsd::userland::libthr {

struct Barrier {
    unsigned count{0};
    unsigned waiting{0};
    unsigned generation{0};
};

[[nodiscard]] inline Status barrier_init(Barrier& b, unsigned count) noexcept {
    if (count == 0) {
        return Status::Invalid;
    }
    b.count = count;
    b.waiting = 0;
    b.generation = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status barrier_wait(Barrier& b, unsigned& generation_out) noexcept {
    ++b.waiting;
    if (b.waiting < b.count) {
        generation_out = b.generation;
        return Status::Busy;
    }
    b.waiting = 0;
    ++b.generation;
    generation_out = b.generation;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
