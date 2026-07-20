module;

#include <cstddef>

export module pbsd.userland.libc.gen.parsetime;

export import pbsd.core;

/// at(1) time token table from hbsd/src/usr.bin/at/parsetime.c
export namespace pbsd::userland::libc {

enum class TimeToken : int {
    Midnight = 1,
    Noon,
    Teatime,
    Pm,
    Am,
    Tomorrow,
    Today,
    Now,
    Minutes,
    Hours,
    Days,
    Weeks,
    Months,
    Years,
    Jan,
    Feb,
    Mar,
    Apr,
    May,
    Jun,
    Jul,
    Aug,
    Sep,
    Oct,
    Nov,
    Dec,
    Sun,
    Mon,
    Tue,
    Wed,
    Thu,
    Fri,
    Sat,
    Unknown = 0,
};

struct TimeKeyword {
    const char* name;
    TimeToken token;
};

inline constexpr TimeKeyword kTimeKeywords[] = {
    {"midnight", TimeToken::Midnight},
    {"noon", TimeToken::Noon},
    {"teatime", TimeToken::Teatime},
    {"am", TimeToken::Am},
    {"pm", TimeToken::Pm},
    {"tomorrow", TimeToken::Tomorrow},
    {"today", TimeToken::Today},
    {"now", TimeToken::Now},
    {"minute", TimeToken::Minutes},
    {"minutes", TimeToken::Minutes},
    {"hour", TimeToken::Hours},
    {"hours", TimeToken::Hours},
    {"day", TimeToken::Days},
    {"days", TimeToken::Days},
    {"week", TimeToken::Weeks},
    {"weeks", TimeToken::Weeks},
    {"month", TimeToken::Months},
    {"months", TimeToken::Months},
    {"year", TimeToken::Years},
    {"years", TimeToken::Years},
    {"jan", TimeToken::Jan},
    {"feb", TimeToken::Feb},
    {"mar", TimeToken::Mar},
    {"apr", TimeToken::Apr},
    {"may", TimeToken::May},
    {"jun", TimeToken::Jun},
    {"jul", TimeToken::Jul},
    {"aug", TimeToken::Aug},
    {"sep", TimeToken::Sep},
    {"oct", TimeToken::Oct},
    {"nov", TimeToken::Nov},
    {"dec", TimeToken::Dec},
};

namespace detail {
[[nodiscard]] inline char to_lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

[[nodiscard]] inline bool ieq_prefix(const char* a, const char* b, std::size_t n) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] == '\0' || b[i] == '\0') {
            return false;
        }
        if (to_lower(a[i]) != to_lower(b[i])) {
            return false;
        }
    }
    return true;
}
} // namespace detail

[[nodiscard]] inline TimeToken lookup_time_keyword(const char* word) noexcept {
    if (word == nullptr || *word == '\0') {
        return TimeToken::Unknown;
    }
    for (const auto& kw : kTimeKeywords) {
        std::size_t len = 0;
        while (kw.name[len] != '\0') {
            ++len;
        }
        if (detail::ieq_prefix(word, kw.name, len) && word[len] == '\0') {
            return kw.token;
        }
    }
    return TimeToken::Unknown;
}

[[nodiscard]] inline unsigned seconds_for_token(TimeToken tok) noexcept {
    switch (tok) {
    case TimeToken::Midnight:
        return 0;
    case TimeToken::Noon:
        return 12U * 3600U;
    case TimeToken::Teatime:
        return 16U * 3600U;
    default:
        return 0;
    }
}

} // namespace pbsd::userland::libc
