module;
#include <cstdint>

export module pbsd.net.altq;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/altq/altq.h — ALTQ discipline identifiers.
export namespace pbsd::net::altq {

enum class Discipline : unsigned char {
    None    = 0,
    Cbq     = 1,
    Priq    = 2,
    Hfsc    = 3,
    Faq     = 4,
    Codel   = 5,
    FqCodel = 6,
};

struct DiscEntry {
    Discipline  id{};
    const char* name{};
};

inline constexpr DiscEntry kDiscTable[] = {
    {Discipline::Cbq, "CBQ"},
    {Discipline::Priq, "PRIQ"},
    {Discipline::Hfsc, "HFSC"},
    {Discipline::Codel, "CODEL"},
    {Discipline::FqCodel, "FQ_CODEL"},
};

[[nodiscard]] inline constexpr std::size_t disc_table_size() noexcept {
    return sizeof(kDiscTable) / sizeof(kDiscTable[0]);
}

[[nodiscard]] inline Status validate_disc(Discipline d) noexcept {
    if (d == Discipline::None) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::altq
