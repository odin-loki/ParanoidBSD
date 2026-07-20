module;

export module pbsd.kernel.kern_sig;

export import pbsd.core;
import pbsd.kernel.signal;

/// Freestanding port of `kern/kern_sig.c` — signal delivery flags.
export namespace pbsd::kernel::kern_sig {

inline constexpr int kSigCantIgn = 0x0001;
inline constexpr int kSigReset = 0x0002;
inline constexpr int kSigCore = 0x0004;

[[nodiscard]] inline bool cant_ignore(int flags) noexcept {
    return (flags & kSigCantIgn) != 0;
}

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_sig
