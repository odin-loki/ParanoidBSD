module;
#include <cstdint>

export module pbsd.stand.cdboot;

import pbsd.core;
import pbsd.stand.cd9660;

/// PROVENANCE: hbsd/src/stand/i386/cdboot/cdboot.S — El Torito no-emulation CD boot.
export namespace pbsd::stand::cdboot {

inline constexpr unsigned kMemPageSize = 0x1000;
inline constexpr unsigned kMemArg = 0x900;
inline constexpr unsigned kMemArgBtx = 0xa100;
inline constexpr unsigned kMemArgSize = 0x18;
inline constexpr unsigned kMemBtxAddress = 0x9000;
inline constexpr unsigned kMemBtxEntry = 0x9010;
inline constexpr unsigned kMemBtxClient = 0xa000;
inline constexpr unsigned kSectorSize = cd9660::kBlockSize;
inline constexpr unsigned kSectorShift = cd9660::kBlockShift;
inline constexpr unsigned kBufferLen = 0x100;
inline constexpr unsigned kMaxRead = 0x10000;
inline constexpr unsigned kNumRetries = 3;
inline constexpr unsigned kErrorTimeout = 0x80;

inline constexpr char kLoaderPath[] = "/boot/loader";

enum class BootStage : unsigned char {
    ArgsBuilt = 0,
    LoaderRead = 1,
    BtxHandoff = 2,
};

struct CdbootParams {
    std::uint32_t boot_drive{};
    std::uint32_t cd_lba{};
    std::uint32_t sector_count{};
    BootStage stage{BootStage::ArgsBuilt};
};

[[nodiscard]] inline Status validate_params(const CdbootParams& p) noexcept {
    if (p.sector_count == 0 || p.sector_count > kMaxRead / kSectorSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t lba_byte_offset(std::uint32_t lba) noexcept {
    return cd9660::lba_to_byte_offset(lba);
}

} // namespace pbsd::stand::cdboot
