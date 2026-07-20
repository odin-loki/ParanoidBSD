module;

export module pbsd.kernel.mac_socket;

import pbsd.core;

/// Freestanding port of `security/mac/mac_socket.c` — socket MAC checks.
export namespace pbsd::kernel::mac_socket {

enum class Op : unsigned char {
    Bind = 0,
    Connect = 1,
    Send = 2,
    Receive = 3,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Bind:
    case Op::Connect:
    case Op::Send:
    case Op::Receive:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status authorize(Op op, bool allowed) noexcept {
    if (validate_op(op) != Status::Ok) {
        return Status::Invalid;
    }
    return allowed ? Status::Ok : Status::Denied;
}

} // namespace pbsd::kernel::mac_socket
