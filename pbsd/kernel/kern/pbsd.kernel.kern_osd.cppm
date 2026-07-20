module;

export module pbsd.kernel.kern_osd;

export import pbsd.core;

/// Freestanding port of `kern/kern_osd.c` — osd helpers.
export namespace pbsd::kernel::kern_osd {

inline constexpr unsigned kOsdSlots = 16;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kOsdSlots ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_osd
