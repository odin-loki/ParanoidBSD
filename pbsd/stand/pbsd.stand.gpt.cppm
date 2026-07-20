module;
#include <cstdint>

export module pbsd.stand.gpt;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/lib/gpt.h — GPT partition type GUIDs (subset).
export namespace pbsd::stand::gpt {

inline constexpr unsigned kHeaderSize = 92;
inline constexpr unsigned kEntrySize  = 128;
inline constexpr unsigned kSignature  = 0x54524150; // "PART"

enum class EntryType : unsigned int {
    EfiSystem   = 0,
    EfiBoot     = 1,
    BiosBoot    = 2,
    Freebsd     = 3,
    FreebsdBoot = 4,
    FreebsdSwap = 5,
    FreebsdZfs  = 6,
    LinuxData   = 7,
    LinuxSwap   = 8,
};

struct Header {
    unsigned signature{};
    unsigned revision{};
    unsigned header_size{};
    unsigned header_crc32{};
    unsigned long long current_lba{};
    unsigned long long backup_lba{};
    unsigned long long first_usable_lba{};
    unsigned long long last_usable_lba{};
};

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.signature != kSignature) {
        return Status::Invalid;
    }
    if (hdr.header_size < kHeaderSize) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::gpt
