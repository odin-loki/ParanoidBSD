module;

#include <cctype>
#include <cstring>

export module pbsd.userland.util.login_times;

import pbsd.userland.libc.string.copy;

/// parse_lt/in_ltm from hbsd/src/lib/libutil/login_times.c
export namespace pbsd::userland::util {

inline constexpr unsigned LC_MAXTIMES = 64;

inline constexpr unsigned char LTM_NONE = 0x00;
inline constexpr unsigned char LTM_SUN  = 0x01;
inline constexpr unsigned char LTM_MON  = 0x02;
inline constexpr unsigned char LTM_TUE  = 0x04;
inline constexpr unsigned char LTM_WED  = 0x08;
inline constexpr unsigned char LTM_THU  = 0x10;
inline constexpr unsigned char LTM_FRI  = 0x20;
inline constexpr unsigned char LTM_SAT  = 0x40;
inline constexpr unsigned char LTM_ANY  = 0x7F;
inline constexpr unsigned char LTM_WK   = 0x3E;
inline constexpr unsigned char LTM_WD   = 0x41;

struct LoginTime {
    unsigned short lt_start{};
    unsigned short lt_end{};
    unsigned char lt_dow{LTM_NONE};
};

struct TmLike {
    int tm_sec{};
    int tm_min{};
    int tm_hour{};
    int tm_wday{};
};

namespace detail {

struct DowSpec {
    const char* dw;
    unsigned char cn;
    unsigned char fl;
};

inline constexpr DowSpec kDows[] = {
    {"su", 2, LTM_SUN}, {"mo", 2, LTM_MON}, {"tu", 2, LTM_TUE},
    {"we", 2, LTM_WED}, {"th", 2, LTM_THU}, {"fr", 2, LTM_FRI},
    {"sa", 2, LTM_SAT}, {"any", 3, LTM_ANY}, {"all", 3, LTM_ANY},
    {"wk", 2, LTM_WK}, {"wd", 2, LTM_WD}, {nullptr, 0, 0},
};

inline char* parse_time(char* ptr, unsigned short* t) noexcept {
    unsigned short val = 0;
    while (*ptr != '\0' && std::isdigit(static_cast<unsigned char>(*ptr)) != 0) {
        val = static_cast<unsigned short>(val * 10 + (*ptr - '0'));
        ++ptr;
    }
    *t = static_cast<unsigned short>((val / 100) * 60 + (val % 100));
    return ptr;
}

} // namespace detail

[[nodiscard]] inline LoginTime parse_lt(const char* str) noexcept {
    LoginTime t{};
    t.lt_dow = LTM_NONE;

    if (str == nullptr || *str == '\0' || std::strcmp(str, "Never") == 0 ||
        std::strcmp(str, "None") == 0) {
        return t;
    }

    LoginTime m = t;
    char buf[64]{};
    pbsd::userland::libc::strlcpy(buf, str, sizeof(buf));
    for (unsigned i = 0; buf[i] != '\0'; ++i) {
        buf[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(buf[i])));
    }
    char* p = buf;

    while (std::isalpha(static_cast<unsigned char>(*p)) != 0) {
        int i = 0;
        while (detail::kDows[i].dw != nullptr &&
               std::strncmp(p, detail::kDows[i].dw, detail::kDows[i].cn) != 0) {
            ++i;
        }
        if (detail::kDows[i].dw == nullptr) {
            break;
        }
        m.lt_dow |= detail::kDows[i].fl;
        p += detail::kDows[i].cn;
    }

    if (m.lt_dow == LTM_NONE) {
        m.lt_dow |= LTM_ANY;
    }

    if (std::isdigit(static_cast<unsigned char>(*p)) != 0) {
        p = detail::parse_time(p, &m.lt_start);
    } else {
        m.lt_start = 0;
    }

    if (*p == '-') {
        p = detail::parse_time(p + 1, &m.lt_end);
    } else {
        m.lt_end = 1440;
    }

    return m;
}

[[nodiscard]] inline int in_ltm(const LoginTime* ltm, const TmLike* tt) noexcept {
    if (tt == nullptr) {
        return 0;
    }

    if ((static_cast<unsigned char>(0x01u << tt->tm_wday) & ltm->lt_dow) != 0) {
        unsigned short now =
            static_cast<unsigned short>(tt->tm_hour * 60 + tt->tm_min);
        if (tt->tm_sec > 30) {
            ++now;
        }
        if (now >= ltm->lt_start && now < ltm->lt_end) {
            return 2;
        }
    }
    return 0;
}

[[nodiscard]] inline int in_ltms(const LoginTime* ltm, const TmLike* tm) noexcept {
    int i = 0;
    while (i < static_cast<int>(LC_MAXTIMES) && ltm[i].lt_dow != LTM_NONE) {
        if (in_ltm(ltm + i, tm) != 0) {
            return i;
        }
        ++i;
    }
    return -1;
}

} // namespace pbsd::userland::util
