export module pbsd.kernel.mbuf;

export import pbsd.core;

/// Wave 4/6 — mbuf flag/type constants from sys/mbuf.h.
export namespace pbsd::kernel::mbuf {

enum class Type : unsigned char {
    Free = 0,
    Data = 1,
    Header = 2,
    Socket = 3,
    Pcb = 4,
    Rtable = 5,
    Htable = 6,
    Atables = 7,
    Soname = 8,
    Soopts = 9,
    Ftable = 10,
    Rights = 11,
    Ifaddr = 12,
};

enum class Flags : unsigned {
    None = 0,
    Ext = 0x0001,      // M_EXT
    Pkthdr = 0x0002,   // M_PKTHDR
    Eor = 0x0004,      // M_EOR
    Rdonly = 0x0008,   // M_RDONLY
    ProtoFlags = 0x0FF0,
    Bcast = 0x0100,    // M_BCAST
    Mcast = 0x0200,    // M_MCAST
    Promisc = 0x0400,  // M_PROMISC
    Vlantage = 0x1000, // M_VLANTAG
};

[[nodiscard]] constexpr Flags operator|(Flags a, Flags b) noexcept {
    return static_cast<Flags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has_flag(Flags set, Flags bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] constexpr Status validate_pkthdr(Flags f) noexcept {
    if (has_flag(f, Flags::Pkthdr) && has_flag(f, Flags::Rdonly)) {
        // readonly pkthdr ok
    }
    return Status::Ok;
}

inline constexpr unsigned kMhlen = 168;   // MHLEN-ish scaffold
inline constexpr unsigned kMlclbytes = 2048;
inline constexpr unsigned kMJumpsize = 4096;

} // namespace pbsd::kernel::mbuf
