module;

export module pbsd.kernel.kern_alq;

export import pbsd.core;

/// Freestanding port of `kern/kern_alq.c` — alq helpers.
export namespace pbsd::kernel::kern_alq {

inline constexpr unsigned kAlqMax = 256;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kAlqMax ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_alq
