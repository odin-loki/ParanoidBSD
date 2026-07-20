module;
#include <cstdint>

export module pbsd.stand.pxe;

import pbsd.core;
import pbsd.stand.net;

/// PROVENANCE: hbsd/src/stand/i386/pxeldr, stand/i386/libi386/pxe.h — PXE network boot.
export namespace pbsd::stand::pxe {

inline constexpr unsigned kMemPageSize = 0x1000;
inline constexpr unsigned kMemArg = 0x900;
inline constexpr unsigned kMemArgBtx = 0xa100;
inline constexpr unsigned kMemArgSize = 0x18;
inline constexpr unsigned kMemBtxAddress = 0x9000;
inline constexpr unsigned kMemBtxEntry = 0x9010;
inline constexpr unsigned kMemBtxClient = 0xa000;
inline constexpr unsigned kBiosEntry = 0x7c00;

inline constexpr char kPxenvSignature[] = "PXENV+";
inline constexpr char kPxeSignature[] = "!PXE";
inline constexpr char kNfsRootPath[] = "/pxeroot";
inline constexpr char kNetifName[] = "pxenet";
inline constexpr char kDefaultFilename[] = "pxeboot";

enum class EnvKind : unsigned char {
    Unknown = 0,
    Pxenv = 1,
    BangPxe = 2,
};

struct PxenvHeader {
    char signature[6]{};
    std::uint16_t version{};
    std::uint8_t length{};
    std::uint8_t checksum{};
};

struct PxeBootParams {
    EnvKind env{EnvKind::Unknown};
    std::uint32_t pxeinfo_lba{};
    net::Proto boot_proto{net::Proto::Bootp};
    char root_path[64]{};
};

[[nodiscard]] inline Status validate_pxenv(const PxenvHeader& hdr) noexcept {
    for (unsigned i = 0; i < 6; ++i) {
        if (hdr.signature[i] != kPxenvSignature[i]) {
            return Status::Invalid;
        }
    }
    if (hdr.length < 16) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_boot_params(const PxeBootParams& p) noexcept {
    if (net::validate_proto(p.boot_proto) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::pxe
