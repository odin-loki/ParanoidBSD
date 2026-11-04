export module pbsd.kernel.gre;

export import pbsd.core;

/// Wave 4 — GRE header flags (net/if_gre.h).
export namespace pbsd::kernel::gre {

enum class Flag : unsigned short {
    ChecksumPresent = 0x8000,
    KeyPresent = 0x2000,
    SequencePresent = 0x1000,
};

inline constexpr unsigned short kFlagsMask =
    static_cast<unsigned short>(Flag::ChecksumPresent)
    | static_cast<unsigned short>(Flag::KeyPresent)
    | static_cast<unsigned short>(Flag::SequencePresent);

struct GreHdr {
    unsigned short flags{};
    unsigned short proto{};
};

[[nodiscard]] constexpr unsigned short sanitize_flags(unsigned short raw) noexcept {
    return raw & kFlagsMask;
}

[[nodiscard]] constexpr Status validate_header(const GreHdr& hdr) noexcept {
    if ((hdr.flags & ~kFlagsMask) != 0) {
        return Status::Invalid;
    }
    if (hdr.proto == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool needs_checksum(unsigned short flags) noexcept {
    return (flags & static_cast<unsigned short>(Flag::ChecksumPresent)) != 0;
}

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return 3;
}

} // namespace pbsd::kernel::gre
