module;
#include <cstdint>

export module pbsd.kernel.acct;

export import pbsd.core;

/// Wave 4/5 — process accounting from sys/acct.h, kern/kern_acct.c.
export namespace pbsd::kernel::acct {

inline constexpr int kAcFlagNormal = 0x01;
inline constexpr int kAcFlagSuser = 0x02;
inline constexpr int kAcFlagCompat = 0x04;
inline constexpr int kAcFlagAsroot = 0x08;
inline constexpr int kAcFlagTerm = 0x10;
inline constexpr int kAcFlagCore = 0x20;
inline constexpr int kAcFlagXexit = 0x40;

inline constexpr int kAcSwsize = 512;
inline constexpr int kAcCommLen = 10;

struct AcctV1 {
    char          ac_flag{};
    char          ac_stat{};
    unsigned char ac_uid{};
    unsigned char ac_gid{};
    unsigned char ac_tty[4]{};
    char          ac_comm[kAcCommLen]{};
    std::uint32_t ac_btime{};
    std::uint32_t ac_etime{};
    std::uint32_t ac_mem{};
    std::uint32_t ac_io{};
    std::uint32_t ac_rw[2]{};
};

struct AcctState {
    bool     enabled{};
    bool     suspended{};
    unsigned interval_sec{60};
};

[[nodiscard]] constexpr bool flag_core(char f) noexcept {
    return (static_cast<unsigned char>(f) & kAcFlagCore) != 0;
}

[[nodiscard]] constexpr bool flag_term(char f) noexcept {
    return (static_cast<unsigned char>(f) & kAcFlagTerm) != 0;
}

[[nodiscard]] constexpr Status validate_interval(unsigned sec) noexcept {
    if (sec == 0 || sec > 86400) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::acct
