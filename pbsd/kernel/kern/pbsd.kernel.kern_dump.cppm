module;

export module pbsd.kernel.kern_dump;

export import pbsd.core;

/// Freestanding port of `kern/kern_dump.c` — dump helpers.
export namespace pbsd::kernel::kern_dump {

inline constexpr unsigned kDumpCompress = 0x0001;
inline constexpr unsigned kDumpLive = 0x0002;

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags & ~(kDumpCompress | kDumpLive)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_dump
