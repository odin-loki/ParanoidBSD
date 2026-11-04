module;
#include <cstdint>

export module pbsd.net.ipfw_rule;

import pbsd.core;
import pbsd.net.ipfw;

/// PROVENANCE: hbsd/src/sys/netinet/ip_fw.h — ipfw(4) match opcodes and actions.
export namespace pbsd::net::ipfw::rule {

enum class MatchOp : unsigned short {
    Nop = 0,
    IpSrc = 1,
    IpDst = 5,
    Proto = 11,
    In = 15,
    Out = 18,
    TcpFlags = 30,
};

enum class ActionOp : unsigned short {
    Accept = 45,
    Deny = 46,
    Divert = 52,
    Tee = 53,
    Skipto = 54,
    Call = 55,
    Return = 56,
};

struct Opcode {
    unsigned short op{};
    unsigned arg1{};
    unsigned arg2{};
};

[[nodiscard]] inline bool is_match(MatchOp op) noexcept {
    return static_cast<unsigned short>(op) <= static_cast<unsigned short>(MatchOp::TcpFlags);
}

[[nodiscard]] inline bool is_action(ActionOp op) noexcept {
    return static_cast<unsigned short>(op) >= static_cast<unsigned short>(ActionOp::Accept);
}

[[nodiscard]] inline Status validate_opcode(Opcode const& oc) noexcept {
    if (oc.op == static_cast<unsigned short>(MatchOp::Nop)) {
        return Status::Ok;
    }
    if (oc.op == static_cast<unsigned short>(ActionOp::Divert)
        || oc.op == static_cast<unsigned short>(ActionOp::Tee)) {
        if (oc.arg1 == 0 || oc.arg1 > 65535) {
            return Status::Invalid;
        }
        return Status::Ok;
    }
    if (oc.op == static_cast<unsigned short>(ActionOp::Skipto)
        || oc.op == static_cast<unsigned short>(ActionOp::Call)) {
        return ipfw::validate_rule_num(oc.arg1);
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rule_and_set(unsigned num, unsigned set) noexcept {
    if (ipfw::validate_rule_num(num) != Status::Ok) {
        return Status::Invalid;
    }
    return ipfw::validate_set(set);
}

enum class DivertFlag : unsigned {
    Loop = 1u << 0,
    Out = 1u << 1,
};

[[nodiscard]] inline bool divert_is_outbound(unsigned bitmap) noexcept {
    return (bitmap & static_cast<unsigned>(DivertFlag::Out)) != 0;
}

} // namespace pbsd::net::ipfw::rule
