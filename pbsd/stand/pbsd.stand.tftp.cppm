module;
#include <cstdint>

export module pbsd.stand.tftp;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/tftp.c, tftp.h — TFTP boot transfer opcodes.
export namespace pbsd::stand::tftp {

inline constexpr unsigned kPort = 69;
inline constexpr unsigned kBlockSize = 512;
inline constexpr unsigned kMaxBlock = 65535;

enum class Opcode : unsigned short {
    Rrq = 1,
    Wrq = 2,
    Data = 3,
    Ack = 4,
    Error = 5,
};

[[nodiscard]] inline Status validate_opcode(Opcode op) noexcept {
    switch (op) {
    case Opcode::Rrq:
    case Opcode::Wrq:
    case Opcode::Data:
    case Opcode::Ack:
    case Opcode::Error:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_block(unsigned block) noexcept {
    return block <= kMaxBlock ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::tftp
