module;

export module pbsd.bifrost.inst_emul;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/vmm_instruction_emul.h
export namespace pbsd::bifrost::inst_emul {

enum class OpType : unsigned char {
    None = 0,
    Mov,
    Movsx,
    Movzx,
    And,
    Or,
    Sub,
    TwoByte,
    Push,
    Cmp,
    Pop,
    Movs,
    Group1,
    Stos,
    Bittest,
    TwobGrp15,
    Add,
    Test,
    Bextr,
    Outs,
};

[[nodiscard]] inline Status validate_op(OpType op) noexcept {
    if (op == OpType::None) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_memory_op(OpType op) noexcept {
    switch (op) {
    case OpType::Mov:
    case OpType::Movs:
    case OpType::Stos:
        return true;
    default:
        return false;
    }
}

} // namespace pbsd::bifrost::inst_emul
