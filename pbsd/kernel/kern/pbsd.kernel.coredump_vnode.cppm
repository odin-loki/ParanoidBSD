module;

export module pbsd.kernel.coredump_vnode;

import pbsd.core;

/// Freestanding port of `kern/coredump_vnode.c` — coredump vnode helpers.
export namespace pbsd::kernel::coredump_vnode {

enum class State : unsigned char {
    Idle = 0,
    Writing = 1,
    Done = 2,
    Failed = 3,
};

struct Ctx {
    State state{State::Idle};
    bool vnode_open{};
};

[[nodiscard]] inline Status begin(Ctx& ctx) noexcept {
    if (ctx.state != State::Idle) {
        return Status::Busy;
    }
    ctx.state = State::Writing;
    return Status::Ok;
}

[[nodiscard]] inline Status finish(Ctx& ctx, bool ok) noexcept {
    if (ctx.state != State::Writing) {
        return Status::Invalid;
    }
    ctx.state = ok ? State::Done : State::Failed;
    return Status::Ok;
}

} // namespace pbsd::kernel::coredump_vnode
