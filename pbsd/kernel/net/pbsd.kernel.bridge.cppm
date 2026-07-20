export module pbsd.kernel.bridge;

export import pbsd.core;

/// Wave 4 — bridge ioctls and STP params (net/if_bridgevar.h).
export namespace pbsd::kernel::bridge {

enum class Cmd : unsigned {
    Add = 0,
    Del = 1,
    GetIfFlags = 2,
    SetIfFlags = 3,
    SetCache = 4,
    GetCache = 5,
    GetIfs = 6,
    Flush = 12,
    SetProto = 28,
};

enum class IfFlag : unsigned {
    Learning = 0x0001,
    Discover = 0x0002,
    Stp = 0x0004,
    StpEdge = 0x0008,
    StpAutoEdge = 0x0010,
    StpPtp = 0x0020,
    StpAutoEdgeConf = 0x0040,
    StpAutoPtpConf = 0x0080,
};

struct Ifbreq {
    unsigned ifbr_ifs{};
    unsigned ifbr_ifsflags{};
    unsigned ifbr_portno{};
    unsigned ifbr_cost{};
    unsigned ifbr_priority{};
};

[[nodiscard]] constexpr bool has(IfFlag set, IfFlag bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] constexpr Status validate_portno(unsigned port) noexcept {
    if (port > 4095) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_ifbreq(const Ifbreq& req) noexcept {
    return validate_portno(req.ifbr_portno);
}

[[nodiscard]] inline unsigned cmd_table_size() noexcept {
    return 10;
}

} // namespace pbsd::kernel::bridge
