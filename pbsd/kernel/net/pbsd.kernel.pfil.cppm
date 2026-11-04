export module pbsd.kernel.pfil;

export import pbsd.core;

/// Wave 4 — packet filter hooks (net/pfil.h).
export namespace pbsd::kernel::pfil {

inline constexpr unsigned kVersion = 2;
inline constexpr unsigned kIn = 0x00010000;
inline constexpr unsigned kOut = 0x00020000;
inline constexpr unsigned kFwd = 0x00040000;
inline constexpr unsigned kDirMask = kIn | kOut | kFwd;

enum class Type : unsigned {
    Ip4 = 0,
    Ip6 = 1,
    Ethernet = 2,
};

enum class Action : int {
    Pass = 0,
    Dropped = 1,
    Consumed = 2,
    Realloced = 3,
};

struct HeadStub {
    unsigned nhooks_in{};
    unsigned nhooks_out{};
};

[[nodiscard]] constexpr unsigned dir_of(unsigned flags) noexcept {
    return flags & kDirMask;
}

[[nodiscard]] constexpr bool hooked_in(const HeadStub& h) noexcept {
    return h.nhooks_in > 0;
}

[[nodiscard]] constexpr bool hooked_out(const HeadStub& h) noexcept {
    return h.nhooks_out > 0;
}

[[nodiscard]] constexpr Status validate_flags(unsigned flags) noexcept {
    if (dir_of(flags) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned action_table_size() noexcept {
    return 4;
}

} // namespace pbsd::kernel::pfil
