module;
#include <cstdint>

export module pbsd.stand.part;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/part.c, sys/diskmbr.h — MBR/GPT partition schemes.
export namespace pbsd::stand::part {

inline constexpr unsigned kMbrParts = 4;
inline constexpr unsigned kGptMaxEntries = 128;
inline constexpr unsigned kGptHeaderLba = 1;
inline constexpr unsigned char kMbrSig0 = 0x55;
inline constexpr unsigned char kMbrSig1 = 0xAA;

enum class Scheme : unsigned char {
    Unknown = 0,
    Mbr = 1,
    Gpt = 2,
    Bsd = 3,
};

enum class MbrType : unsigned char {
    Freebsd = 0xA5,
    FreebsdSlice = 0xA6,
    Efi = 0xEE,
};

[[nodiscard]] inline Status validate_scheme(Scheme s) noexcept {
    return s == Scheme::Unknown ? Status::Invalid : Status::Ok;
}

[[nodiscard]] inline Status validate_mbr_signature(unsigned char b0,
                                                   unsigned char b1) noexcept {
    return b0 == kMbrSig0 && b1 == kMbrSig1 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::part
