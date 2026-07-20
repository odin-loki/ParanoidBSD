module;
#include <cstdint>

export module pbsd.zfs.zap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zap.h — ZAP name/value limits.
export namespace pbsd::zfs::zap {

inline constexpr unsigned kMaxNameLen = 256;
inline constexpr unsigned kMaxValueLen = 8192;
inline constexpr unsigned kChunkGap = 24;

enum class Type : unsigned char {
    Micro = 0,
    Fat   = 1,
};

enum class MatchType : unsigned {
    Normalize = 1u << 0,
    MatchCase = 1u << 1,
};

enum class Flag : unsigned {
    None = 0,
    Hash64 = 1u << 0,
    Uint64Key = 1u << 1,
    PreHashedKey = 1u << 2,
};

[[nodiscard]] constexpr Flag operator|(Flag a, Flag b) noexcept {
    return static_cast<Flag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline Type type_for_name_len(unsigned len) noexcept {
    return len <= 49 ? Type::Micro : Type::Fat;
}

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kMaxNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_value_len(unsigned len) noexcept {
    if (len > kMaxValueLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_entry(unsigned name_len, unsigned value_len) noexcept {
    if (validate_name_len(name_len) != Status::Ok) {
        return Status::Invalid;
    }
    return validate_value_len(value_len);
}

} // namespace pbsd::zfs::zap
