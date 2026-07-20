module;

export module pbsd.kernel.mac_inet;

import pbsd.core;

/// Freestanding port of `security/mac/mac_inet.c` — IPv4 MAC checks.
export namespace pbsd::kernel::mac_inet {

enum class Op : unsigned char {
    Send = 0,
    Receive = 1,
    Rebind = 2,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    switch (op) {
    case Op::Send:
    case Op::Receive:
    case Op::Rebind:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status check_packet(Op op, bool allowed) noexcept {
    if (validate_op(op) != Status::Ok) {
        return Status::Invalid;
    }
    return allowed ? Status::Ok : Status::Denied;
}

} // namespace pbsd::kernel::mac_inet
