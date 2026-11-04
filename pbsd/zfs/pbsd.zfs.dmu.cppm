module;
#include <cstdint>

export module pbsd.zfs.dmu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dmu.h — DMU object types.
export namespace pbsd::zfs::dmu {

inline constexpr unsigned char kOtNewtype = 0x80;
inline constexpr unsigned char kOtMetadata = 0x40;
inline constexpr unsigned char kOtEncrypted = 0x20;
inline constexpr unsigned char kOtByteswapMask = 0x1f;
inline constexpr unsigned long long kMaxAccess = 1ull << 17;

enum class Byteswap : unsigned char {
    Uint8 = 0,
    Uint16 = 1,
    Uint32 = 2,
    Uint64 = 3,
    Zap = 4,
    Dnode = 5,
    Objset = 6,
    Znode = 7,
    OldAcl = 8,
    Acl = 9,
    NumFuncs = 10,
};

enum class ObjsetType : unsigned char {
    None = 0,
    Meta = 1,
    Zil = 2,
    Dnode = 3,
    Other = 4,
};

enum class TxType : unsigned char {
    Read = 0,
    Write = 1,
    Free = 2,
    Claim = 3,
};

[[nodiscard]] constexpr unsigned char make_object_type(Byteswap swap, bool metadata,
                                                       bool encrypted) noexcept {
    unsigned char ot = kOtNewtype;
    if (metadata) {
        ot |= kOtMetadata;
    }
    if (encrypted) {
        ot |= kOtEncrypted;
    }
    ot |= static_cast<unsigned char>(swap) & kOtByteswapMask;
    return ot;
}

[[nodiscard]] inline bool object_type_valid(unsigned char ot) noexcept {
    if ((ot & kOtNewtype) == 0) {
        return ot < static_cast<unsigned char>(Byteswap::NumFuncs);
    }
    return (ot & kOtByteswapMask) < static_cast<unsigned char>(Byteswap::NumFuncs);
}

[[nodiscard]] inline Status validate_objset_type(ObjsetType t) noexcept {
    if (static_cast<unsigned char>(t) > static_cast<unsigned char>(ObjsetType::Other)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_tx(TxType tx, unsigned long long size) noexcept {
    if (tx == TxType::Write && size > kMaxAccess) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dmu
