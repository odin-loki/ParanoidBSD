module;

export module pbsd.kernel.kern_ktr;

export import pbsd.core;

/// Freestanding port of `kern/kern_ktr.c` — ktr helpers.
export namespace pbsd::kernel::kern_ktr {

inline constexpr unsigned kKtrGen = 0x0001;

[[nodiscard]] inline Status validate_class(unsigned cls) noexcept {
    return cls <= 255 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_ktr
