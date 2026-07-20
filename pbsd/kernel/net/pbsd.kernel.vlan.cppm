export module pbsd.kernel.vlan;

export import pbsd.core;

/// Wave 4 — VLAN tag helpers (net/if_vlan_var.h).
export namespace pbsd::kernel::vlan {

inline constexpr unsigned short kVlidMask = 0x0FFF;
inline constexpr unsigned short kPriMask = 0xE000;
inline constexpr unsigned short kCfiMask = 0x1000;

[[nodiscard]] constexpr unsigned short vlan_id(unsigned short tag) noexcept {
    return tag & kVlidMask;
}

[[nodiscard]] constexpr unsigned short priority(unsigned short tag) noexcept {
    return (tag & kPriMask) >> 13;
}

[[nodiscard]] constexpr unsigned short make_tag(unsigned short vlid,
                                                unsigned char pri,
                                                bool cfi = false) noexcept {
    const unsigned short cfi_bit = cfi ? kCfiMask : 0;
    return static_cast<unsigned short>((static_cast<unsigned>(pri) << 13) | cfi_bit | (vlid & kVlidMask));
}

[[nodiscard]] constexpr Status validate_vlid(unsigned short vlid) noexcept {
    if (vlid == 0 || vlid > kVlidMask) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_priority(unsigned char pri) noexcept {
    if (pri > 7) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct TagStub {
    unsigned short ether_vtag{};
    bool has_tag{};
};

[[nodiscard]] constexpr Status apply_vlid(TagStub& m, unsigned short vlid) noexcept {
    if (validate_vlid(vlid) != Status::Ok) {
        return Status::Invalid;
    }
    if (m.has_tag) {
        m.ether_vtag = static_cast<unsigned short>((m.ether_vtag & ~kVlidMask) | vlid);
    } else {
        m.ether_vtag = vlid;
        m.has_tag = true;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vlan
