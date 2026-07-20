export module pbsd.net.vlan;

export import pbsd.core;

/// Wave 6 — 802.1Q VLAN constants from net/if_vlan_var.h.
export namespace pbsd::net::vlan {

inline constexpr unsigned kMaxVid = 4094;
inline constexpr unsigned kNullVid = 0;
inline constexpr unsigned kPriorityMax = 7;
inline constexpr unsigned kHeaderLen = 4;
inline constexpr unsigned short kEtherType = 0x8100;

struct Tag {
    unsigned short vid{0}; // 12-bit
    unsigned char pcp{0};  // 3-bit
    bool dei{false};       // drop eligible
};

[[nodiscard]] constexpr Status validate_vid(unsigned vid) noexcept {
    if (vid == kNullVid || vid > kMaxVid) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr unsigned short encode_tci(Tag t) noexcept {
    unsigned short v = static_cast<unsigned short>(t.vid & 0x0fff);
    v |= static_cast<unsigned short>((t.pcp & 0x7) << 13);
    if (t.dei) {
        v |= 0x1000;
    }
    return v;
}

[[nodiscard]] constexpr Tag decode_tci(unsigned short tci) noexcept {
    Tag t{};
    t.vid = static_cast<unsigned short>(tci & 0x0fff);
    t.pcp = static_cast<unsigned char>((tci >> 13) & 0x7);
    t.dei = (tci & 0x1000) != 0;
    return t;
}

} // namespace pbsd::net::vlan
