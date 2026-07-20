module;
#include <cstdint>

export module pbsd.fs.isofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/isofs/cd9660/iso.h — ISO9660 primary volume descriptor.
export namespace pbsd::fs::isofs {

inline constexpr unsigned kSectorSize = 2048;
inline constexpr unsigned char kVdPrimary = 1;
inline constexpr unsigned char kVdSupplementary = 2;
inline constexpr unsigned char kVdTerminator = 255;

struct VolumeDesc {
    unsigned char type{kVdPrimary};
    char standard_id[5]{'C', 'D', '0', '0', '1'};
    unsigned char version{1};
};

[[nodiscard]] inline Status validate(const VolumeDesc& v) noexcept {
    if (v.version == 0) {
        return Status::Invalid;
    }
    if (v.standard_id[0] != 'C' || v.standard_id[1] != 'D') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::isofs
