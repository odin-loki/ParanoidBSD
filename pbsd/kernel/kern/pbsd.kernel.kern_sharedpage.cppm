module;

export module pbsd.kernel.kern_sharedpage;

export import pbsd.core;

/// Freestanding port of `kern/kern_sharedpage.c` — sharedpage helpers.
export namespace pbsd::kernel::kern_sharedpage {

inline constexpr unsigned kSpPresent = 0x0001;
inline constexpr unsigned kSpMapped = 0x0002;

struct State {
    unsigned flags{};
    unsigned gen{};
};

[[nodiscard]] inline Status publish(State& s) noexcept {
    ++s.gen;
    s.flags |= kSpPresent;
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_sharedpage
