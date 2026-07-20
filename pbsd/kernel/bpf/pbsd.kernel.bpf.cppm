export module pbsd.kernel.bpf;

export import pbsd.core;

/// Wave 4 — BPF ioctl/filter constants from net/bpf.h.
export namespace pbsd::kernel::bpf {

enum class Mode : unsigned {
    Writable = 0,
    ReadOnly = 1,
    WriteOnly = 2,
};

enum class InsnClass : unsigned char {
    Ld = 0x00,
    Ldx = 0x01,
    St = 0x02,
    Stx = 0x03,
    Alu = 0x04,
    Jmp = 0x05,
    Ret = 0x06,
    Misc = 0x07,
};

enum class Size : unsigned char {
    W = 0x00,
    H = 0x08,
    B = 0x10,
};

struct Insn {
    unsigned short code{};
    unsigned char jt{};
    unsigned char jf{};
    unsigned k{};
};

inline constexpr unsigned kMaxInsns = 512;
inline constexpr unsigned kAlignment = 8;

[[nodiscard]] constexpr Status validate_program(const Insn* prog, unsigned n) noexcept {
    if (prog == nullptr || n == 0 || n > kMaxInsns) {
        return Status::Invalid;
    }
    // Last insn should be RET in a well-formed filter
    const unsigned short cls = static_cast<unsigned short>(prog[n - 1].code & 0x07);
    if (cls != static_cast<unsigned short>(InsnClass::Ret)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

struct TapStub {
    Mode mode{Mode::ReadOnly};
    unsigned buffer_len{4096};
    unsigned snap_len{65535};
};

[[nodiscard]] constexpr Status validate_tap(const TapStub& tap) noexcept {
    if (tap.buffer_len == 0 || tap.snap_len == 0) {
        return Status::Invalid;
    }
    if (tap.snap_len > 65535) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned mode_table_size() noexcept {
    return 3;
}

} // namespace pbsd::kernel::bpf
