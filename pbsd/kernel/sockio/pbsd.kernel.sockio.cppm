module;
#include <cstdint>

export module pbsd.kernel.sockio;

export import pbsd.core;

/// Wave 4 — socket/ioctl commands (sys/sys/sockio.h).
export namespace pbsd::kernel::sockio {

inline constexpr unsigned kIoWrite = 0x80000000u;
inline constexpr unsigned kIoRead  = 0x40000000u;
inline constexpr unsigned kIo      = 0x80000000u;

inline constexpr unsigned kSiocshiwat = (kIoWrite | (sizeof(int) << 16) | ('s' << 8) | 0);
inline constexpr unsigned kSiocghiwat = (kIoRead  | (sizeof(int) << 16) | ('s' << 8) | 1);
inline constexpr unsigned kSiocslowat = (kIoWrite | (sizeof(int) << 16) | ('s' << 8) | 2);
inline constexpr unsigned kSiocglowat = (kIoRead  | (sizeof(int) << 16) | ('s' << 8) | 3);
inline constexpr unsigned kSiocsifflags = (kIoWrite | (256u << 16) | ('i' << 8) | 16);
inline constexpr unsigned kSiocgifflags = (kIoRead  | (256u << 16) | ('i' << 8) | 17);

struct IoctlEntry {
    unsigned    cmd{};
    const char* name{};
    bool        ifnet_class{};
};

inline constexpr IoctlEntry kIoctlTable[] = {
    {kSiocshiwat,   "SIOCSHIWAT",   false},
    {kSiocghiwat,   "SIOCGHIWAT",   false},
    {kSiocsifflags, "SIOCSIFFLAGS", true},
    {kSiocgifflags, "SIOCGIFFLAGS", true},
};

[[nodiscard]] inline unsigned ioctl_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kIoctlTable) / sizeof(kIoctlTable[0]));
}

[[nodiscard]] constexpr bool is_ifnet_ioctl(unsigned cmd) noexcept {
    return cmd == kSiocsifflags || cmd == kSiocgifflags;
}

[[nodiscard]] constexpr Status validate_ioctl(unsigned cmd) noexcept {
    for (const auto& e : kIoctlTable) {
        if (e.cmd == cmd) {
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

} // namespace pbsd::kernel::sockio
