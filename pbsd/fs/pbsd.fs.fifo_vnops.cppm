module;

export module pbsd.fs.fifo_vnops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fifofs — FIFO vnode ops scaffold.
export namespace pbsd::fs::fifo_vnops {

enum class Op : unsigned char {
    Open = 0,
    Close = 1,
    Read = 2,
    Write = 3,
    Poll = 4,
    Kqfilter = 5,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Open:
    case Op::Close:
    case Op::Read:
    case Op::Write:
    case Op::Poll:
    case Op::Kqfilter:
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline bool is_io(Op op) noexcept {
    return op == Op::Read || op == Op::Write;
}

} // namespace pbsd::fs::fifo_vnops
