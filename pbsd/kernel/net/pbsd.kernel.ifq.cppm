module;

export module pbsd.kernel.ifq;

import pbsd.core;

/// Freestanding port of `net/ifq.c` / `net/ifq.h` — driver ring enqueue helpers.
export namespace pbsd::kernel::ifq {

struct QueueStub {
    unsigned length{};
    unsigned max_len{256};
    unsigned drops{};
    bool     altq_enabled{};
};

[[nodiscard]] inline Status enqueue(QueueStub& q) noexcept {
    if (q.length >= q.max_len) {
        ++q.drops;
        return Status::Denied;
    }
    ++q.length;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(QueueStub& q) noexcept {
    if (q.length == 0) {
        return Status::Invalid;
    }
    --q.length;
    return Status::Ok;
}

[[nodiscard]] inline Status prepend(QueueStub& q) noexcept {
    if (q.length >= q.max_len) {
        ++q.drops;
        return Status::Denied;
    }
    ++q.length;
    return Status::Ok;
}

[[nodiscard]] inline unsigned length(const QueueStub& q) noexcept {
    return q.length;
}

inline void flush(QueueStub& q) noexcept {
    q.length = 0;
}

} // namespace pbsd::kernel::ifq
