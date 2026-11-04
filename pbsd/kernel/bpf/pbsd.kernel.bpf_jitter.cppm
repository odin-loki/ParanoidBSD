module;
#include <cstdint>

export module pbsd.kernel.bpf_jitter;

import pbsd.core;

/// Freestanding port of `net/bpf_jitter.c` — BPF JIT compile surface.
export namespace pbsd::kernel::bpf_jitter {

inline constexpr unsigned kMaxInsns = 512;

struct Program {
    unsigned insn_count{};
    bool     compiled{};
    void*    native_code{nullptr};
};

[[nodiscard]] inline Status validate(const Program& prog) noexcept {
    if (prog.insn_count == 0 || prog.insn_count > kMaxInsns) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status compile(Program& prog) noexcept {
    if (validate(prog) != Status::Ok) {
        return Status::Invalid;
    }
    if (prog.compiled) {
        return Status::Busy;
    }
    prog.compiled = true;
    prog.native_code = reinterpret_cast<void*>(1);
    return Status::Ok;
}

[[nodiscard]] inline Status release(Program& prog) noexcept {
    if (!prog.compiled) {
        return Status::Invalid;
    }
    prog.compiled = false;
    prog.native_code = nullptr;
    return Status::Ok;
}

[[nodiscard]] inline bool is_compiled(const Program& prog) noexcept {
    return prog.compiled;
}

} // namespace pbsd::kernel::bpf_jitter
