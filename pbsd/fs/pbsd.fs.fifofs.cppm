module;

export module pbsd.fs.fifofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fifofs — FIFO vnode operations scaffold.
export namespace pbsd::fs::fifofs {

enum class Op : unsigned char {
    Open = 0,
    Read = 1,
    Write = 2,
    Poll = 3,
    Close = 4,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Open:
    case Op::Read:
    case Op::Write:
    case Op::Poll:
    case Op::Close:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_reader_blocked(unsigned readers, unsigned buffer_free) noexcept {
    return readers > 0 && buffer_free == 0;
}

} // namespace pbsd::fs::fifofs
