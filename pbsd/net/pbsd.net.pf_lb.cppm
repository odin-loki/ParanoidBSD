module;
#include <cstdint>

export module pbsd.net.pf_lb;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netpfil/pf/pf_lb.c — Pf Lb scaffold.
export namespace pbsd::net::pf_lb {

enum class Op : unsigned char {
    Init = 0,
    Validate = 1,
    Dispatch = 2,
};

struct Ctx {
    unsigned flags{};
    unsigned count{};
    bool active{false};
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Init:
    case Op::Validate:
    case Op::Dispatch:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status init(Ctx& ctx) noexcept {
    if (ctx.active) {
        return Status::Busy;
    }
    ctx.active = true;
    ctx.count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status dispatch(Ctx& ctx, Op op) noexcept {
    if (validate_op(op) != Status::Ok) {
        return Status::Invalid;
    }
    if (!ctx.active) {
        return Status::Invalid;
    }
    ++ctx.count;
    return Status::Ok;
}

} // namespace pbsd::net::pf_lb
