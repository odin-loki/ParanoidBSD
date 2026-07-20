export module pbsd.kernel.if_media;

export import pbsd.core;

/// Wave 4 — ifmedia types/options (net/if_media.h).
export namespace pbsd::kernel::if_media {

inline constexpr unsigned kTypeMask = 0x000000E0;
inline constexpr unsigned kSubtypeMask = 0x0000001F;
inline constexpr unsigned kEther = 0x00000020;

enum class Subtype : unsigned {
    Auto = 0,
    TenT = 3,
    HundredTx = 6,
    ThousandT = 16,
    TenGlr = 18,
};

enum class Option : unsigned {
    FullDuplex = 0x00100000,
    HalfDuplex = 0x00200000,
    Flow = 0x00400000,
    Loop = 0x00800000,
};

struct MediaStub {
    unsigned ifm_current{};
    unsigned ifm_mask{};
};

[[nodiscard]] constexpr unsigned ether_subtype(unsigned ifm) noexcept {
    return (ifm & kSubtypeMask) | ((ifm & 0x0000F000) >> 4);
}

[[nodiscard]] constexpr Status validate_media(unsigned ifm) noexcept {
    if ((ifm & kTypeMask) != kEther) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned subtype_table_size() noexcept {
    return 5;
}

} // namespace pbsd::kernel::if_media
