module;

export module pbsd.kernel.mac_vfs;

import pbsd.core;

/// Freestanding port of `security/mac/mac_vfs.c` — VFS MAC checks.
export namespace pbsd::kernel::mac_vfs {

enum class Op : unsigned char {
    Lookup = 0,
    Create = 1,
    Unlink = 2,
    Rename = 3,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Lookup:
    case Op::Create:
    case Op::Unlink:
    case Op::Rename:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status check_vnode(Op op, bool allowed) noexcept {
    if (validate_op(op) != Status::Ok) {
        return Status::Invalid;
    }
    return allowed ? Status::Ok : Status::Denied;
}

} // namespace pbsd::kernel::mac_vfs
