module;

export module pbsd.kernel.kern_jaildesc;

export import pbsd.core;

/// Freestanding port of `kern/kern_jaildesc.c` — jaildesc helpers.
export namespace pbsd::kernel::kern_jaildesc {

inline constexpr unsigned kJdNameMax = 256;

[[nodiscard]] inline Status validate_jid(int jid) noexcept {
    return jid >= 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_jaildesc
