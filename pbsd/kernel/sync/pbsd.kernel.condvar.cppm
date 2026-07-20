module;

export module pbsd.kernel.condvar;

export import pbsd.core;

/// Wave 5 — condition variable stubs from sys/condvar.h.
export namespace pbsd::kernel::condvar {

struct CondvarStub {
    int waiters{};
};

[[nodiscard]] constexpr bool has_waiters(const CondvarStub& cv) noexcept {
    return cv.waiters > 0;
}

[[nodiscard]] constexpr Status validate_waiters(int n) noexcept {
    if (n < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status signal(CondvarStub& cv) noexcept {
    if (cv.waiters <= 0) {
        return Status::Ok;
    }
    --cv.waiters;
    return Status::Ok;
}

[[nodiscard]] inline Status broadcast(CondvarStub& cv) noexcept {
    if (cv.waiters < 0) {
        return Status::Invalid;
    }
    cv.waiters = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status wait_enqueue(CondvarStub& cv) noexcept {
    if (cv.waiters < 0) {
        return Status::Invalid;
    }
    ++cv.waiters;
    return Status::Ok;
}

} // namespace pbsd::kernel::condvar
