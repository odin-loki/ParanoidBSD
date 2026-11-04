module;
#include <cstdint>

export module pbsd.stand.bios;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/i386/libi386/bios*.c — BIOS boot service constants.
export namespace pbsd::stand::bios {

inline constexpr unsigned kIntDisk = 0x13;
inline constexpr unsigned kIntVideo = 0x10;
inline constexpr unsigned kIntMem = 0x15;
inline constexpr unsigned kSectorSize = 512;
inline constexpr unsigned kMaxCylinders = 1024;
inline constexpr unsigned kMaxHeads = 255;
inline constexpr unsigned kMaxSectors = 63;

enum class DiskOp : unsigned char {
    Reset = 0x00,
    Read = 0x02,
    Write = 0x03,
    ExtRead = 0x42,
    ExtWrite = 0x43,
    GetDriveParams = 0x48,
};

struct Chs {
    unsigned cylinder{0};
    unsigned head{0};
    unsigned sector{1};
};

[[nodiscard]] inline Status validate_chs(const Chs& c) noexcept {
    if (c.cylinder >= kMaxCylinders || c.head >= kMaxHeads) {
        return Status::Invalid;
    }
    if (c.sector == 0 || c.sector > kMaxSectors) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status lba_to_chs(std::uint64_t lba, unsigned heads,
                                       unsigned sec_per_track, Chs& out) noexcept {
    if (heads == 0 || sec_per_track == 0) {
        return Status::Invalid;
    }
    const auto spc = static_cast<std::uint64_t>(heads) * sec_per_track;
    out.cylinder = static_cast<unsigned>(lba / spc);
    const auto rem = lba % spc;
    out.head = static_cast<unsigned>(rem / sec_per_track);
    out.sector = static_cast<unsigned>(rem % sec_per_track) + 1;
    return validate_chs(out);
}

} // namespace pbsd::stand::bios
