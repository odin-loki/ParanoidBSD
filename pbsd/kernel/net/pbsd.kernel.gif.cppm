export module pbsd.kernel.gif;

export import pbsd.core;

/// Wave 4 — GIF tunnel flags (net/if_gif.h).
export namespace pbsd::kernel::gif {

enum class Flag : unsigned {
    Link0 = 0x1000,
    Link1 = 0x2000,
    Link2 = 0x4000,
    AcceptRtadv = 0x8000,
    IgnoreSource = 0x10000,
    IgnoreSourceRouting = 0x20000,
    Pmtudisc = 0x40000,
    IgnoreSourceRoutingStrict = 0x80000,
};

enum class Family : int {
    None = 0,
    Inet = 2,
    Inet6 = 28,
};

struct SoftcStub {
    Family family{Family::None};
    unsigned flags{};
    unsigned fibnum{};
    unsigned options{};
    void* netgraph{};
};

[[nodiscard]] constexpr Flag operator|(Flag a, Flag b) noexcept {
    return static_cast<Flag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has(Flag set, Flag bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] constexpr Status validate_softc(const SoftcStub& sc) noexcept {
    if (sc.family == Family::None && sc.flags != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return 7;
}

} // namespace pbsd::kernel::gif
