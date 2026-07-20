module;

export module pbsd.kernel.if_dead;

import pbsd.core;

/// Freestanding port of `net/if_dead.c` — detached ifnet stub handlers.
export namespace pbsd::kernel::if_dead {

enum class Handler : unsigned {
    Output,
    Input,
    Start,
    Ioctl,
    ResolveMulti,
};

struct IfnetStub {
    bool detached{};
    unsigned drops{};
};

[[nodiscard]] inline Status output(IfnetStub& ifp) noexcept {
    if (!ifp.detached) {
        return Status::Invalid;
    }
    ++ifp.drops;
    return Status::NotFound;
}

[[nodiscard]] inline Status input(IfnetStub& ifp) noexcept {
    if (!ifp.detached) {
        return Status::Invalid;
    }
    ++ifp.drops;
    return Status::NotFound;
}

[[nodiscard]] inline Status ioctl(IfnetStub& ifp) noexcept {
    if (!ifp.detached) {
        return Status::Invalid;
    }
    return Status::NotFound;
}

[[nodiscard]] inline Status mark_detached(IfnetStub& ifp) noexcept {
    if (ifp.detached) {
        return Status::Busy;
    }
    ifp.detached = true;
    return Status::Ok;
}

[[nodiscard]] inline bool is_detached(const IfnetStub& ifp) noexcept {
    return ifp.detached;
}

} // namespace pbsd::kernel::if_dead
