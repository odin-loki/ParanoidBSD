module;
#include <cstdint>

export module pbsd.fs.cd9660;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/cd9660/iso.h — ISO9660 volume descriptor types.
export namespace pbsd::fs::cd9660 {

inline constexpr unsigned char kVdPrimary = 1;
inline constexpr unsigned char kVdSupplementary = 2;
inline constexpr unsigned char kVdEnd = 255;

inline constexpr char kStandardId[] = "CD001";
inline constexpr char kEcmaId[] = "CDW01";

enum class VdType : unsigned char {
    Primary = kVdPrimary,
    Supplementary = kVdSupplementary,
    End = kVdEnd,
};

[[nodiscard]] inline Status validate_vd_type(VdType t) noexcept {
    switch (t) {
    case VdType::Primary:
    case VdType::Supplementary:
    case VdType::End:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::cd9660
