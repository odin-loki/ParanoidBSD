module;
#include <cstdint>

export module pbsd.net.mld6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/mld6.h — MLDv2 query/report helpers.
export namespace pbsd::net::mld6 {

inline constexpr unsigned kMinLen = 24;
inline constexpr unsigned kV2QueryMinLen = 28;
inline constexpr unsigned kV2ReportMaxRecs = 65535;
inline constexpr unsigned kV1MaxRi = 10;
inline constexpr unsigned kTimerScale = 1000;

enum class RecordMode : unsigned char {
    DoNothing           = 0,
    ModeIsInclude       = 1,
    ModeIsExclude       = 2,
    ChangeToInclude     = 3,
    ChangeToExclude     = 4,
    AllowNewSources     = 5,
    BlockOldSources     = 6,
};

enum class QueryType : unsigned char {
    General       = 1,
    Group         = 2,
    GroupSource   = 3,
};

struct V2Query {
    unsigned char type{};
    unsigned char code{};
    unsigned short checksum{};
    unsigned char misc{};
    unsigned char qqi{};
    unsigned short numsrc{};
};

struct V2Record {
    unsigned char type{};
    unsigned char datalen{};
    unsigned short numsrc{};
};

[[nodiscard]] inline unsigned mrc_exp(unsigned short mrc) noexcept {
    return (mrc >> 12) & 0x0007;
}

[[nodiscard]] inline unsigned mrc_mant(unsigned short mrc) noexcept {
    return mrc & 0x0fff;
}

[[nodiscard]] inline unsigned qqic_exp(unsigned char qqic) noexcept {
    return (qqic >> 4) & 0x07;
}

[[nodiscard]] inline unsigned qqic_mant(unsigned char qqic) noexcept {
    return qqic & 0x0f;
}

[[nodiscard]] inline bool suppress_flag(unsigned char misc) noexcept {
    return ((misc >> 3) & 0x01) != 0;
}

[[nodiscard]] inline unsigned querier_robustness(unsigned char misc) noexcept {
    return misc & 0x07;
}

[[nodiscard]] inline Status validate_record_mode(RecordMode mode) noexcept {
    if (static_cast<unsigned char>(mode) > static_cast<unsigned char>(RecordMode::BlockOldSources)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_query(V2Query const& q) noexcept {
    if (q.numsrc > kV2ReportMaxRecs) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_record(V2Record const& rec) noexcept {
    if (validate_record_mode(static_cast<RecordMode>(rec.type)) != Status::Ok) {
        return Status::Invalid;
    }
    if (rec.numsrc > kV2ReportMaxRecs) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::mld6
