module;
#include <cstdint>

export module pbsd.geom.eli;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/eli/g_eli.h, g_eli_key.c, g_eli_key_cache.c,
/// g_eli_hmac.c — GEOM ELI encryption flags, keys, and IV schedule.
export namespace pbsd::geom::eli {

inline constexpr char kClassName[] = "ELI";
inline constexpr char kMagic[] = "GEOM::ELI";
inline constexpr char kSuffix[] = ".eli";

inline constexpr unsigned kVersion = 7;
inline constexpr unsigned kMaxKeys = 2;
inline constexpr unsigned kMaxKeyLen = 64;
inline constexpr unsigned kDataKeyLen = kMaxKeyLen;
inline constexpr unsigned kIvKeyLen = kMaxKeyLen;
inline constexpr unsigned kAuthKeyLen = kMaxKeyLen;
inline constexpr unsigned kSaltLen = 64;
inline constexpr unsigned kSha512MacLen = 64;
inline constexpr unsigned kSha256DigestLen = 32;
inline constexpr unsigned kAuthSecKeyLen = kSha256DigestLen;
inline constexpr unsigned kDataIvKeyLen = kDataKeyLen + kIvKeyLen;
inline constexpr unsigned kMasterKeyLen = kDataIvKeyLen + kSha512MacLen;
inline constexpr unsigned kKeyShift = 20;
inline constexpr unsigned kOverwrites = 5;
inline constexpr int kKeyMagic = 0xe11341c;

inline constexpr unsigned kAesBlockLen = 16;
inline constexpr unsigned kAesXtsIvLen = 8;
inline constexpr unsigned kCamelliaBlockLen = 16;

inline constexpr unsigned char kHmacSaltDerived0 = 0x00;
inline constexpr unsigned char kHmacSaltDerived1 = 0x01;
inline constexpr unsigned char kHmacSaltEncKey = 0x10;
inline constexpr unsigned char kHmacSaltAuthKey = 0x11;

enum class Algo : unsigned char {
    None = 0,
    AesXts = 1,
    AesCbc = 2,
    CamelliaCbc = 3,
};

enum class DiskFlag : unsigned int {
    Onetime = 0x00000001,
    Boot = 0x00000002,
    Auth = 0x00000010,
    Readonly = 0x00000020,
    Geliboot = 0x00000080,
};

enum class RuntimeFlag : unsigned int {
    NativeByteOrder = 0x00040000,
    SingleKey = 0x00080000,
    FirstKey = 0x00200000,
    EncIvKey = 0x00400000,
};

enum class HmacPurpose : unsigned char {
    MasterVerify = 0,
    MasterEncrypt = 1,
    EncryptionKey = 0x10,
    AuthenticationKey = 0x11,
};

[[nodiscard]] inline Status validate_algo(Algo a) noexcept {
    if (a == Algo::None) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned iv_length(Algo algo) noexcept {
    switch (algo) {
    case Algo::AesXts:
        return kAesXtsIvLen;
    case Algo::AesCbc:
        return kAesBlockLen;
    case Algo::CamelliaCbc:
        return kCamelliaBlockLen;
    default:
        return 0;
    }
}

[[nodiscard]] inline unsigned key_bit_length(Algo algo, unsigned requested) noexcept {
    switch (algo) {
    case Algo::None:
        if (requested == 0) {
            return 64 * 8;
        }
        return requested > 64 * 8 ? 0 : requested;
    case Algo::AesCbc:
    case Algo::CamelliaCbc:
        if (requested == 0) {
            return 128;
        }
        switch (requested) {
        case 128:
        case 192:
        case 256:
            return requested;
        default:
            return 0;
        }
    case Algo::AesXts:
        if (requested == 0) {
            return 128;
        }
        switch (requested) {
        case 128:
        case 256:
            return requested;
        default:
            return 0;
        }
    default:
        return 0;
    }
}

[[nodiscard]] inline unsigned runtime_flags_for_version(unsigned version,
                                                        unsigned disk_flags) noexcept {
    unsigned flags = disk_flags;
    if (version < 4) {
        flags |= static_cast<unsigned>(RuntimeFlag::NativeByteOrder);
    }
    if (version < 5) {
        flags |= static_cast<unsigned>(RuntimeFlag::SingleKey);
    }
    if (version < 6 && (flags & static_cast<unsigned>(DiskFlag::Auth)) != 0) {
        flags |= static_cast<unsigned>(RuntimeFlag::FirstKey);
    }
    if (version < 7) {
        flags |= static_cast<unsigned>(RuntimeFlag::EncIvKey);
    }
    return flags;
}

[[nodiscard]] inline bool uses_single_key(unsigned runtime_flags) noexcept {
    return (runtime_flags & static_cast<unsigned>(RuntimeFlag::SingleKey)) != 0;
}

[[nodiscard]] inline bool uses_enc_ivkey(unsigned runtime_flags) noexcept {
    return (runtime_flags & static_cast<unsigned>(RuntimeFlag::EncIvKey)) != 0;
}

[[nodiscard]] inline bool uses_native_byte_order(unsigned runtime_flags) noexcept {
    return (runtime_flags & static_cast<unsigned>(RuntimeFlag::NativeByteOrder)) != 0;
}

[[nodiscard]] inline unsigned key_slot_bit(unsigned slot) noexcept {
    return 1u << slot;
}

[[nodiscard]] inline bool key_slot_present(unsigned key_mask, unsigned slot) noexcept {
    return (key_mask & key_slot_bit(slot)) != 0;
}

[[nodiscard]] inline unsigned master_key_slot_offset(unsigned slot) noexcept {
    return slot * kMasterKeyLen;
}

struct MasterKeyLayout {
    unsigned iv_key_offset{};
    unsigned data_key_offset{};
    unsigned on_disk_hmac_offset{};
};

[[nodiscard]] inline MasterKeyLayout master_key_layout() noexcept {
    return MasterKeyLayout{0, kIvKeyLen, kDataIvKeyLen};
}

struct MasterKeyView {
    unsigned char const* iv_key{};
    unsigned char const* data_key{};
    unsigned char const* on_disk_hmac{};
};

[[nodiscard]] inline MasterKeyView view_master_key(unsigned char const* master) noexcept {
    if (master == nullptr) {
        return {};
    }
    auto layout = master_key_layout();
    return MasterKeyView{
        master + layout.iv_key_offset,
        master + layout.data_key_offset,
        master + layout.on_disk_hmac_offset,
    };
}

struct KeyFillPayload {
    char magic[4]{'e', 'k', 'e', 'y'};
    unsigned char keyno[8]{};
};

[[nodiscard]] inline Status build_key_fill_payload(unsigned long long keyno,
                                                   KeyFillPayload& out) noexcept {
    out.magic[0] = 'e';
    out.magic[1] = 'k';
    out.magic[2] = 'e';
    out.magic[3] = 'y';
    for (unsigned i = 0; i < 8; ++i) {
        out.keyno[i] = static_cast<unsigned char>((keyno >> (8 * i)) & 0xff);
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned char const* encryption_key_source(
    unsigned runtime_flags,
    unsigned char const* master_key,
    unsigned char const* derived_ekey) noexcept {
    if (uses_enc_ivkey(runtime_flags)) {
        return master_key;
    }
    return derived_ekey;
}

struct EncryptionKeyEntry {
    unsigned char key[kDataKeyLen]{};
    unsigned long long keyno{};
    int magic{};
};

[[nodiscard]] inline Status init_encryption_key_entry(EncryptionKeyEntry& entry,
                                                      unsigned long long keyno) noexcept {
    entry.keyno = keyno;
    entry.magic = kKeyMagic;
    return Status::Ok;
}

[[nodiscard]] inline unsigned key_index_for_offset(unsigned long long sector_offset,
                                                   unsigned sectors_per_key) noexcept {
    if (sectors_per_key == 0) {
        return 0;
    }
    return static_cast<unsigned>(sector_offset / sectors_per_key);
}

[[nodiscard]] inline unsigned sectors_per_key(unsigned sector_size) noexcept {
    (void)sector_size;
    return static_cast<unsigned>(1ull << kKeyShift);
}

[[nodiscard]] inline unsigned long long key_number(unsigned long long byte_offset,
                                                   unsigned blocksize) noexcept {
    if (blocksize == 0) {
        return 0;
    }
    return (byte_offset >> kKeyShift) / blocksize;
}

[[nodiscard]] inline unsigned long long total_key_count(unsigned long long mediasize,
                                                        unsigned blocksize) noexcept {
    if (blocksize == 0 || mediasize == 0) {
        return 0;
    }
    return ((mediasize - 1) >> kKeyShift) / blocksize + 1;
}

struct KeySchedule {
    unsigned char master[kMasterKeyLen]{};
    unsigned char derived_ekey[kDataKeyLen]{};
    unsigned char iv_key[kIvKeyLen]{};
    unsigned active_index{};
    unsigned sectors_per_key{};
    unsigned runtime_flags{};
    Algo algo{Algo::AesXts};
};

[[nodiscard]] inline Status init_key_schedule(KeySchedule& ks,
                                              unsigned sector_size,
                                              unsigned runtime_flags = 0,
                                              Algo algo = Algo::AesXts) noexcept {
    if (sector_size == 0) {
        return Status::Invalid;
    }
    ks.sectors_per_key = sectors_per_key(sector_size);
    ks.active_index = 0;
    ks.runtime_flags = runtime_flags;
    ks.algo = algo;
    return Status::Ok;
}

[[nodiscard]] inline unsigned select_key_index(KeySchedule const& ks,
                                               unsigned long long byte_offset,
                                               unsigned sector_size) noexcept {
    if (uses_single_key(ks.runtime_flags)) {
        return 0;
    }
    if (sector_size == 0) {
        return 0;
    }
    unsigned long long sector = byte_offset / sector_size;
    return key_index_for_offset(static_cast<unsigned>(sector), ks.sectors_per_key)
        % kMaxKeys;
}

[[nodiscard]] inline unsigned long long select_key_number(KeySchedule const& ks,
                                                          unsigned long long byte_offset,
                                                          unsigned blocksize) noexcept {
    if (uses_single_key(ks.runtime_flags)) {
        return 0;
    }
    return key_number(byte_offset, blocksize);
}

struct IvSchedule {
    unsigned runtime_flags{};
    Algo algo{Algo::AesXts};
};

[[nodiscard]] inline Status init_iv_schedule(IvSchedule& ivs,
                                           unsigned runtime_flags,
                                           Algo algo) noexcept {
    if (validate_algo(algo) != Status::Ok) {
        return Status::Invalid;
    }
    ivs.runtime_flags = runtime_flags;
    ivs.algo = algo;
    return Status::Ok;
}

[[nodiscard]] inline void store_le64(unsigned char out[8], unsigned long long value) noexcept {
    for (unsigned i = 0; i < 8; ++i) {
        out[i] = static_cast<unsigned char>((value >> (8 * i)) & 0xff);
    }
}

[[nodiscard]] inline Status encode_iv_offset(unsigned long long offset,
                                             bool native_byte_order,
                                             unsigned char out[8]) noexcept {
    if (out == nullptr) {
        return Status::Invalid;
    }
    if (native_byte_order) {
        for (unsigned i = 0; i < 8; ++i) {
            out[i] = static_cast<unsigned char>((offset >> (8 * i)) & 0xff);
        }
    } else {
        store_le64(out, offset);
    }
    return Status::Ok;
}

[[nodiscard]] inline Status prepare_xts_iv(unsigned long long offset,
                                           IvSchedule const& ivs,
                                           unsigned char* iv,
                                           unsigned iv_size) noexcept {
    if (iv == nullptr || iv_size < kAesXtsIvLen) {
        return Status::Invalid;
    }
    if (ivs.algo != Algo::AesXts) {
        return Status::Invalid;
    }
    unsigned char off[8]{};
    if (encode_iv_offset(offset, uses_native_byte_order(ivs.runtime_flags), off)
        != Status::Ok) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < kAesXtsIvLen; ++i) {
        iv[i] = off[i];
    }
    for (unsigned i = kAesXtsIvLen; i < iv_size; ++i) {
        iv[i] = 0;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status prepare_cbc_iv_offset(unsigned long long offset,
                                                  IvSchedule const& ivs,
                                                  unsigned char out[8]) noexcept {
    if (ivs.algo == Algo::AesXts) {
        return Status::Invalid;
    }
    return encode_iv_offset(offset, uses_native_byte_order(ivs.runtime_flags), out);
}

[[nodiscard]] inline unsigned expected_iv_length(IvSchedule const& ivs) noexcept {
    return iv_length(ivs.algo);
}

[[nodiscard]] inline unsigned char hmac_purpose_salt(HmacPurpose purpose) noexcept {
    return static_cast<unsigned char>(purpose);
}

} // namespace pbsd::geom::eli
