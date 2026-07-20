module;

export module pbsd.fs.pseudofs_vnops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/pseudofs — pseudofs vnode ops scaffold.
export namespace pbsd::fs::pseudofs_vnops {

enum class Op : unsigned char {
    Lookup = 0,
    Open = 1,
    Read = 2,
    Write = 3,
    Readdir = 4,
    Reclaim = 5,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Lookup:
    case Op::Open:
    case Op::Read:
    case Op::Write:
    case Op::Readdir:
    case Op::Reclaim:
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline Status reject_write(Op op) noexcept {
    if (op == Op::Write) {
        return Status::Denied;
    }
    return validate_op(op);
}

} // namespace pbsd::fs::pseudofs_vnops
