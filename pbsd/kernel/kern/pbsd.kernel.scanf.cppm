module;

export module pbsd.kernel.scanf;

import pbsd.core;

/// Freestanding port of `kern/subr_scanf.c` — kernel scanf conversion flags.
export namespace pbsd::kernel::scanf {

inline constexpr unsigned kBuf = 32;

enum class ConvFlag : unsigned {
    Long = 0x01,
    Short = 0x04,
    Suppress = 0x08,
    Pointer = 0x10,
    NoSkip = 0x20,
    Quad = 0x400,
    IntMax = 0x800,
    PtrDiff = 0x1000,
    SizeT = 0x2000,
    ShortShort = 0x4000,
};

enum class ConvType : unsigned {
    Char = 0,
    Ccl = 1,
    String = 2,
    Int = 3,
    Float = 4,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    const bool has_long = (flags & static_cast<unsigned>(ConvFlag::Long)) != 0;
    const bool has_short = (flags & static_cast<unsigned>(ConvFlag::Short)) != 0;
    if (has_long && has_short) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::scanf
