#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 2 burst 12 hand-port userland modules (+23 libc/msun/libthr/usr.bin)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UL = ROOT / "pbsd" / "userland"
CMAKE = UL / "CMakeLists.txt"

MODULES: dict[str, str] = {}


def add(rel: str, body: str) -> None:
    MODULES[rel.replace("\\", "/")] = body.strip() + "\n"


# --- libc ---------------------------------------------------------------------

add(
    "libc/pbsd.userland.libc.gen.fmtcheck.cppm",
    """
module;

#include <cctype>

export module pbsd.userland.libc.gen.fmtcheck;

export import pbsd.core;

/// fmtcheck from hbsd/src/lib/libc/gen/fmtcheck.c (format-string compatibility)
export namespace pbsd::userland::libc {

enum class FmtType : unsigned char {
    Start,
    Short,
    Int,
    Long,
    String,
    Double,
    Unknown,
    Done,
};

[[nodiscard]] inline FmtType next_format_type(const char*& fmt) noexcept {
    if (fmt == nullptr || *fmt == '\\0') {
        return FmtType::Done;
    }
    while (*fmt != '\\0' && *fmt != '%') {
        ++fmt;
    }
    if (*fmt == '\\0') {
        return FmtType::Done;
    }
    ++fmt;
    if (*fmt == '\\0') {
        return FmtType::Unknown;
    }
    while (*fmt == '#' || *fmt == '0' || *fmt == '-' || *fmt == ' ' || *fmt == '+') {
        ++fmt;
    }
    while (*fmt >= '0' && *fmt <= '9') {
        ++fmt;
    }
    if (*fmt == '.') {
        ++fmt;
        while (*fmt >= '0' && *fmt <= '9') {
            ++fmt;
        }
    }
    switch (*fmt) {
    case 'h':
        ++fmt;
        return FmtType::Short;
    case 'l':
        ++fmt;
        return (*fmt == 'l') ? (++fmt, FmtType::Long) : FmtType::Long;
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
    case 'c':
        ++fmt;
        return FmtType::Int;
    case 's':
        ++fmt;
        return FmtType::String;
    case 'f':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
        ++fmt;
        return FmtType::Double;
    case '%':
        ++fmt;
        return FmtType::Start;
    default:
        ++fmt;
        return FmtType::Unknown;
    }
}

[[nodiscard]] inline bool fmt_types_compatible(FmtType a, FmtType b) noexcept {
    if (a == FmtType::Unknown || b == FmtType::Unknown) {
        return false;
    }
    if (a == b) {
        return true;
    }
    if ((a == FmtType::Int && b == FmtType::Short) ||
        (a == FmtType::Short && b == FmtType::Int)) {
        return true;
    }
    return false;
}

[[nodiscard]] inline const char* fmtcheck(const char* fmt, const char* def) noexcept {
    if (fmt == nullptr) {
        return def;
    }
    const char* f = fmt;
    const char* d = def;
    while (true) {
        const FmtType ft = next_format_type(f);
        const FmtType dt = next_format_type(d);
        if (ft == FmtType::Done && dt == FmtType::Done) {
            return fmt;
        }
        if (ft == FmtType::Done || dt == FmtType::Done) {
            return def;
        }
        if (!fmt_types_compatible(ft, dt)) {
            return def;
        }
    }
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.parsetime.cppm",
    """
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
        if (a[i] == '\\0' || b[i] == '\\0') {
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
    if (word == nullptr || *word == '\\0') {
        return TimeToken::Unknown;
    }
    for (const auto& kw : kTimeKeywords) {
        std::size_t len = 0;
        while (kw.name[len] != '\\0') {
            ++len;
        }
        if (detail::ieq_prefix(word, kw.name, len) && word[len] == '\\0') {
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
""",
)

# --- msun ---------------------------------------------------------------------

for _name, _fn, _fnf in [
    ("erf", "std::erf", "std::erff"),
    ("erfc", "std::erfc", "std::erfcf"),
    ("lgamma", "std::lgamma", "std::lgammaf"),
]:
    add(
        f"msun/pbsd.userland.msun.{_name}.cppm",
        f"""
module;

#include <cmath>

export module pbsd.userland.msun.{_name};

/// {_name} from hbsd/src/lib/msun/src/s_{_name}.c
export namespace pbsd::userland::msun {{

[[nodiscard]] inline double {_name}(double x) noexcept {{ return {_fn}(x); }}

[[nodiscard]] inline float {_name}f(float x) noexcept {{ return {_fnf}(x); }}

}} // namespace pbsd::userland::msun
""",
    )

add(
    "msun/pbsd.userland.msun.remquo.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.remquo;

/// remquo from hbsd/src/lib/msun/src/s_remquo.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double remquo(double x, double y, int* quo) noexcept {
    return std::remquo(x, y, quo);
}

[[nodiscard]] inline float remquof(float x, float y, int* quo) noexcept {
    return std::remquof(x, y, quo);
}

} // namespace pbsd::userland::msun
""",
)

# --- libthr -------------------------------------------------------------------

add(
    "libthr/pbsd.userland.libthr.cond_timed.cppm",
    """
module;

export module pbsd.userland.libthr.cond_timed;

import pbsd.core;
import pbsd.userland.libthr.cond;
import pbsd.userland.libthr.mutex;

/// pthread_cond_timedwait from hbsd/src/lib/libthr/thread/thr_cond.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status cond_timedwait(Cond& c, Mutex& m, ThreadId self,
                                         unsigned ticks) noexcept {
    (void)ticks;
    return cond_wait(c, m, self);
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.rwlock_timed.cppm",
    """
module;

export module pbsd.userland.libthr.rwlock_timed;

import pbsd.core;
import pbsd.userland.libthr.rwlock;
import pbsd.userland.libthr.rwlock_try;

/// pthread_rwlock_timed* from hbsd/src/lib/libthr/thread/thr_rwlock.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status rwlock_timedrdlock(RwLock& l, unsigned ticks) noexcept {
    (void)ticks;
    return rwlock_rdlock(l);
}

[[nodiscard]] inline Status rwlock_timedwrlock(RwLock& l, ThreadId self,
                                               unsigned ticks) noexcept {
    (void)ticks;
    return rwlock_trywrlock(l, self);
}

} // namespace pbsd::userland::libthr
""",
)

# --- usr.bin ------------------------------------------------------------------

add(
    "usr.bin/pbsd.userland.biff.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.biff;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/biff/biff.c — tty mail-notification mode bits.
export namespace pbsd::userland::usr_bin::biff {

inline constexpr unsigned kModeNotify = 0x0040U;  /* S_IXUSR */
inline constexpr unsigned kModeBell = 0x0020U;    /* S_IXGRP */

enum class Mode : char { Notify = 'y', Bell = 'b', Off = 'n' };

[[nodiscard]] inline char status_char(unsigned mode) noexcept {
    if (mode & kModeNotify) {
        return 'y';
    }
    if (mode & kModeBell) {
        return 'b';
    }
    return 'n';
}

[[nodiscard]] inline Result<Mode> parse_mode_arg(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\\0' || arg[1] != '\\0') {
        return result_err<Mode>(Status::Invalid);
    }
    switch (arg[0]) {
    case 'y':
        return result_ok(Mode::Notify);
    case 'b':
        return result_ok(Mode::Bell);
    case 'n':
        return result_ok(Mode::Off);
    default:
        return result_err<Mode>(Status::Invalid);
    }
}

[[nodiscard]] inline unsigned apply_mode(unsigned current, Mode mode) noexcept {
    current &= ~(kModeNotify | kModeBell);
    switch (mode) {
    case Mode::Notify:
        return current | kModeNotify;
    case Mode::Bell:
        return current | kModeBell;
    case Mode::Off:
        return current;
    }
    return current;
}

} // namespace pbsd::userland::usr_bin::biff
""",
)

add(
    "usr.bin/pbsd.userland.beep.cppm",
    """
module;

export module pbsd.userland.beep;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/beep/beep.c — tone parameter validation.
export namespace pbsd::userland::usr_bin::beep {

inline constexpr int kSampleRateDef = 48000;
inline constexpr int kSampleRateMax = 48000;
inline constexpr int kSampleRateMin = 8000;
inline constexpr int kDurationDef = 150;
inline constexpr int kDurationMax = 2000;
inline constexpr int kDurationMin = 50;
inline constexpr int kGainDef = 75;
inline constexpr int kGainMax = 100;
inline constexpr int kGainMin = 0;
inline constexpr int kDefaultHz = 440;

[[nodiscard]] inline bool in_range(int v, int lo, int hi) noexcept {
    return v >= lo && v <= hi;
}

[[nodiscard]] inline Result<int> clamp_frequency(int hz) noexcept {
    if (hz <= 0) {
        return result_ok(kDefaultHz);
    }
    if (hz > 20000) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(hz);
}

[[nodiscard]] inline Result<int> clamp_duration(int ms) noexcept {
    if (!in_range(ms, kDurationMin, kDurationMax)) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(ms);
}

[[nodiscard]] inline Result<int> clamp_gain(int gain) noexcept {
    if (!in_range(gain, kGainMin, kGainMax)) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(gain);
}

} // namespace pbsd::userland::usr_bin::beep
""",
)

add(
    "usr.bin/pbsd.userland.morse.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.morse;

export import pbsd.core;

/// Morse table from hbsd/src/usr.bin/morse/morse.c
export namespace pbsd::userland::usr_bin::morse {

struct MorseEntry {
    char inch;
    const char* code;
};

inline constexpr MorseEntry kTable[] = {
    {'a', ".-"},   {'b', "-..."}, {'c', "-.-."}, {'d', "-.."},  {'e', "."},
    {'f', "..-."}, {'g', "--."},  {'h', "...."}, {'i', ".."},   {'j', ".---"},
    {'k', "-.-"},  {'l', ".-.."}, {'m', "--"},   {'n', "-."},   {'o', "---"},
    {'p', ".--."}, {'q', "--.-"}, {'r', ".-."},  {'s', "..."},  {'t', "-"},
    {'u', "..-"},  {'v', "...-"}, {'w', ".--"},  {'x', "-..-"}, {'y', "-.--"},
    {'z', "--.."},
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
    {',', "--..--"}, {'.', ".-.-.-"}, {'?', "..--.."}, {'\\0', ""},
};

[[nodiscard]] inline const char* lookup(char ch) noexcept {
    char lower = ch;
    if (lower >= 'A' && lower <= 'Z') {
        lower = static_cast<char>(lower - 'A' + 'a');
    }
    for (const auto& e : kTable) {
        if (e.inch == lower) {
            return e.code;
        }
    }
    return nullptr;
}

[[nodiscard]] inline std::size_t encode_char(char ch, char* out, std::size_t cap) noexcept {
    const char* code = lookup(ch);
    if (code == nullptr || out == nullptr || cap == 0) {
        return 0;
    }
    std::size_t n = 0;
    while (code[n] != '\\0' && n + 1 < cap) {
        out[n] = code[n];
        ++n;
    }
    if (n < cap) {
        out[n] = '\\0';
    }
    return n;
}

} // namespace pbsd::userland::usr_bin::morse
""",
)

add(
    "usr.bin/pbsd.userland.killall.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.killall;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/killall/killall.c — signal name helpers.
export namespace pbsd::userland::usr_bin::killall {

inline constexpr const char* kSignalNames[] = {
    "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE",
    "KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG",
};

namespace detail {
[[nodiscard]] inline char to_lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}
} // namespace detail

[[nodiscard]] inline int signal_index(const char* name) noexcept {
    if (name == nullptr) {
        return -1;
    }
    for (std::size_t i = 0; i < sizeof(kSignalNames) / sizeof(kSignalNames[0]); ++i) {
        const char* sig = kSignalNames[i];
        std::size_t j = 0;
        while (name[j] != '\\0' && sig[j] != '\\0' &&
               detail::to_lower(name[j]) == detail::to_lower(sig[j])) {
            ++j;
        }
        if (name[j] == '\\0' && sig[j] == '\\0') {
            return static_cast<int>(i + 1);
        }
    }
    return -1;
}

[[nodiscard]] inline Result<int> parse_signal_option(const char* opt) noexcept {
    if (opt == nullptr || *opt != '-') {
        return result_err<int>(Status::Invalid);
    }
    ++opt;
    if (*opt == '\\0') {
        return result_err<int>(Status::Invalid);
    }
    const int idx = signal_index(opt);
    if (idx < 0) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(idx);
}

} // namespace pbsd::userland::usr_bin::killall
""",
)

add(
    "usr.bin/pbsd.userland.enigma.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.enigma;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/enigma/enigma.c — rotor shuffle constants.
export namespace pbsd::userland::usr_bin::enigma {

inline constexpr int kRotorSize = 256;
inline constexpr int kMask = 0377;
inline constexpr int kSeedBase = 123;
inline constexpr int kPrime = 65521;

[[nodiscard]] inline std::uint32_t mix_seed(std::uint32_t seed, unsigned char ch,
                                            int idx) noexcept {
    return seed * 5U + static_cast<std::uint32_t>(ch) + static_cast<std::uint32_t>(idx);
}

[[nodiscard]] inline int shuffle_index(std::uint32_t rnd, int k) noexcept {
    return static_cast<int>((rnd & kMask) % static_cast<unsigned>(k + 1));
}

[[nodiscard]] inline void identity_rotor(unsigned char deck[kRotorSize]) noexcept {
    for (int i = 0; i < kRotorSize; ++i) {
        deck[i] = static_cast<unsigned char>(i);
    }
}

} // namespace pbsd::userland::usr_bin::enigma
""",
)

add(
    "usr.bin/pbsd.userland.ipcrm.cppm",
    """
module;

export module pbsd.userland.ipcrm;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/ipcrm/ipcrm.c — SysV IPC object type letters.
export namespace pbsd::userland::usr_bin::ipcrm {

enum class IpcKind : char { MessageQueue = 'q', Semaphore = 's', SharedMem = 'm' };

[[nodiscard]] inline Result<IpcKind> parse_kind(char flag) noexcept {
    switch (flag) {
    case 'q':
    case 'Q':
        return result_ok(IpcKind::MessageQueue);
    case 's':
    case 'S':
        return result_ok(IpcKind::Semaphore);
    case 'm':
    case 'M':
        return result_ok(IpcKind::SharedMem);
    default:
        return result_err<IpcKind>(Status::Invalid);
    }
}

[[nodiscard]] inline const char* kind_string(IpcKind k) noexcept {
    switch (k) {
    case IpcKind::MessageQueue:
        return "msqid";
    case IpcKind::Semaphore:
        return "semid";
    case IpcKind::SharedMem:
        return "shmid";
    }
    return "unknown";
}

} // namespace pbsd::userland::usr_bin::ipcrm
""",
)

add(
    "usr.bin/pbsd.userland.ipcs.cppm",
    """
module;

export module pbsd.userland.ipcs;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/ipcs/ipcs.c — IPC listing format flags.
export namespace pbsd::userland::usr_bin::ipcs {

enum class ListFlag : unsigned {
    None = 0,
    MessageQueues = 1U << 0,
    Semaphores = 1U << 1,
    SharedMem = 1U << 2,
    All = MessageQueues | Semaphores | SharedMem,
};

[[nodiscard]] inline ListFlag operator|(ListFlag a, ListFlag b) noexcept {
    return static_cast<ListFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline bool has_flag(ListFlag set, ListFlag bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0U;
}

[[nodiscard]] inline Result<ListFlag> parse_list_option(char opt) noexcept {
    switch (opt) {
    case 'q':
        return result_ok(ListFlag::MessageQueues);
    case 's':
        return result_ok(ListFlag::Semaphores);
    case 'm':
        return result_ok(ListFlag::SharedMem);
    case 'a':
        return result_ok(ListFlag::All);
    default:
        return result_err<ListFlag>(Status::Invalid);
    }
}

} // namespace pbsd::userland::usr_bin::ipcs
""",
)

add(
    "usr.bin/pbsd.userland.getent.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.getent;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/getent/getent.c — nss database dispatch table.
export namespace pbsd::userland::usr_bin::getent {

enum class Database : int {
    Ethers,
    Group,
    Hosts,
    Netgroup,
    Networks,
    Passwd,
    Protocols,
    Rpc,
    Services,
    Shells,
    Utmpx,
    Unknown,
};

struct DatabaseEntry {
    const char* name;
    Database id;
};

inline constexpr DatabaseEntry kDatabases[] = {
    {"ethers", Database::Ethers},     {"group", Database::Group},
    {"hosts", Database::Hosts},       {"netgroup", Database::Netgroup},
    {"networks", Database::Networks},   {"passwd", Database::Passwd},
    {"protocols", Database::Protocols}, {"rpc", Database::Rpc},
    {"services", Database::Services},   {"shells", Database::Shells},
    {"utmpx", Database::Utmpx},
};

[[nodiscard]] inline Database lookup_database(const char* name) noexcept {
    if (name == nullptr) {
        return Database::Unknown;
    }
    for (const auto& e : kDatabases) {
        if (hosted::cstrcmp(name, e.name) == 0) {
            return e.id;
        }
    }
    return Database::Unknown;
}

[[nodiscard]] inline const char* database_name(Database db) noexcept {
    for (const auto& e : kDatabases) {
        if (e.id == db) {
            return e.name;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::usr_bin::getent
""",
)

add(
    "usr.bin/pbsd.userland.showmount.cppm",
    """
module;

export module pbsd.userland.showmount;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/showmount/showmount.c — RPC mount listing flags.
export namespace pbsd::userland::usr_bin::showmount {

enum class ShowFlag : unsigned {
    None = 0,
    Dump = 1U << 0,
    Exports = 1U << 1,
    ParsableExports = 1U << 2,
};

[[nodiscard]] inline ShowFlag operator|(ShowFlag a, ShowFlag b) noexcept {
    return static_cast<ShowFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline Result<ShowFlag> parse_show_option(char opt) noexcept {
    switch (opt) {
    case 'd':
        return result_ok(ShowFlag::Dump);
    case 'e':
        return result_ok(ShowFlag::Exports);
    case 'p':
        return result_ok(ShowFlag::ParsableExports);
    default:
        return result_err<ShowFlag>(Status::Invalid);
    }
}

} // namespace pbsd::userland::usr_bin::showmount
""",
)

add(
    "usr.bin/pbsd.userland.finger.cppm",
    """
module;

export module pbsd.userland.finger;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/finger/finger.c — output format flags.
export namespace pbsd::userland::usr_bin::finger {

struct Options {
    bool long_format{false};
    bool no_plan{false};
    bool suppress{false};
    bool mail_status{false};
    bool host_field{true};
};

[[nodiscard]] inline Result<Options> parse_short_option(char opt) noexcept {
    Options o{};
    switch (opt) {
    case 'l':
        o.long_format = true;
        return result_ok(o);
    case 's':
        o.suppress = true;
        return result_ok(o);
    case 'm':
        o.mail_status = true;
        return result_ok(o);
    case 'h':
        o.host_field = true;
        return result_ok(o);
    case 'o':
        o.host_field = false;
        return result_ok(o);
    case 'p':
        o.no_plan = true;
        return result_ok(o);
    default:
        return result_err<Options>(Status::Invalid);
    }
}

[[nodiscard]] inline bool looks_like_phone_extension(const char* s) noexcept {
    if (s == nullptr) {
        return false;
    }
    int digits = 0;
    for (const char* p = s; *p != '\\0'; ++p) {
        if (*p >= '0' && *p <= '9') {
            ++digits;
        } else if (*p != ' ' && *p != '-') {
            return false;
        }
    }
    return digits == 4;
}

} // namespace pbsd::userland::usr_bin::finger
""",
)

add(
    "usr.bin/pbsd.userland.vacation.cppm",
    """
module;

export module pbsd.userland.vacation;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/vacation/vacation.c — autoreply message limits.
export namespace pbsd::userland::usr_bin::vacation {

inline constexpr int kMaxSubject = 256;
inline constexpr int kMaxFrom = 256;

[[nodiscard]] inline bool login_char_ok(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}

[[nodiscard]] inline Result<int> validate_login(const char* login) noexcept {
    if (login == nullptr || *login == '\\0') {
        return result_err<int>(Status::Invalid);
    }
    for (const char* p = login; *p != '\\0'; ++p) {
        if (!login_char_ok(*p)) {
            return result_err<int>(Status::Invalid);
        }
    }
    return result_ok(0);
}

[[nodiscard]] inline bool subject_present(const char* subject) noexcept {
    return subject != nullptr && *subject != '\\0';
}

} // namespace pbsd::userland::usr_bin::vacation
""",
)

add(
    "usr.bin/pbsd.userland.unifdef.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.unifdef;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/unifdef/unifdef.c — directive prefix scan.
export namespace pbsd::userland::usr_bin::unifdef {

enum class Directive { If, Ifdef, Ifndef, Elif, Else, Endif, Unknown };

[[nodiscard]] inline Directive classify_directive(const char* line) noexcept {
    if (line == nullptr) {
        return Directive::Unknown;
    }
    if (line[0] != '#' || line[1] != ' ') {
        return Directive::Unknown;
    }
    const char* p = line + 2;
    struct Pair {
        const char* word;
        Directive dir;
    };
    static constexpr Pair kPairs[] = {
        {"if ", Directive::If},       {"ifdef ", Directive::Ifdef},
        {"ifndef ", Directive::Ifndef}, {"elif ", Directive::Elif},
        {"else", Directive::Else},    {"endif", Directive::Endif},
    };
    for (const auto& pair : kPairs) {
        std::size_t i = 0;
        while (pair.word[i] != '\\0' && p[i] == pair.word[i]) {
            ++i;
        }
        if (pair.word[i] == '\\0') {
            return pair.dir;
        }
    }
    return Directive::Unknown;
}

} // namespace pbsd::userland::usr_bin::unifdef
""",
)

add(
    "usr.bin/pbsd.userland.at.cppm",
    """
module;

export module pbsd.userland.at;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/at/at.c — job queue path constants.
export namespace pbsd::userland::usr_bin::at {

inline constexpr const char* kJobDir = "/var/spool/cron/atjobs";
inline constexpr const char* kLockFile = "/var/spool/cron/atjobs/.lockfile";

namespace detail {
[[nodiscard]] inline bool ieq(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    while (*a != '\\0' && *b != '\\0') {
        char ca = *a;
        char cb = *b;
        if (ca >= 'A' && ca <= 'Z') {
            ca = static_cast<char>(ca - 'A' + 'a');
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = static_cast<char>(cb - 'A' + 'a');
        }
        if (ca != cb) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}
} // namespace detail

[[nodiscard]] inline Result<unsigned> parse_relative_minutes(const char* spec) noexcept {
    if (spec == nullptr || *spec != '+') {
        return result_err<unsigned>(Status::Invalid);
    }
    ++spec;
    unsigned val = 0;
    for (; *spec >= '0' && *spec <= '9'; ++spec) {
        val = val * 10U + static_cast<unsigned>(*spec - '0');
    }
    if (val == 0) {
        return result_err<unsigned>(Status::Invalid);
    }
    const bool hours = detail::ieq(spec, "hours") || detail::ieq(spec, "hour");
    const bool mins = detail::ieq(spec, "minutes") || detail::ieq(spec, "minute");
    if (!hours && !mins) {
        return result_err<unsigned>(Status::Invalid);
    }
    if (hours) {
        val *= 60U;
    }
    return result_ok(val);
}

} // namespace pbsd::userland::usr_bin::at
""",
)

add(
    "usr.bin/pbsd.userland.ministat.cppm",
    """
module;

export module pbsd.userland.ministat;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/ministat/ministat.c — Student-t confidence table.
export namespace pbsd::userland::usr_bin::ministat {

inline constexpr int kNStudent = 100;
inline constexpr int kNConf = 6;
inline constexpr double kStudentPct[kNConf] = {80, 90, 95, 98, 99, 99.5};

[[nodiscard]] inline double student_lookup(int df, int conf_idx) noexcept {
    if (df < 0 || df > kNStudent || conf_idx < 0 || conf_idx >= kNConf) {
        return 0.0;
    }
    static constexpr double kInfRow[kNConf] = {1.282, 1.645, 1.960, 2.326, 2.576, 3.090};
    if (df == 0) {
        return kInfRow[conf_idx];
    }
    static constexpr double kRow1[kNConf] = {3.078, 6.314, 12.706, 31.821, 63.657, 318.313};
    if (df == 1) {
        return kRow1[conf_idx];
    }
    return kInfRow[conf_idx];
}

[[nodiscard]] inline double mean(const double* vals, int n) noexcept {
    if (vals == nullptr || n <= 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += vals[i];
    }
    return sum / static_cast<double>(n);
}

} // namespace pbsd::userland::usr_bin::ministat
""",
)


LIBC_NEW = [
    "libc/pbsd.userland.libc.gen.fmtcheck.cppm",
    "libc/pbsd.userland.libc.gen.parsetime.cppm",
    "libc/pbsd.userland.libc.gen.getpeereid.cppm",
]

MSUN_NEW = [
    "msun/pbsd.userland.msun.erf.cppm",
    "msun/pbsd.userland.msun.erfc.cppm",
    "msun/pbsd.userland.msun.lgamma.cppm",
    "msun/pbsd.userland.msun.remquo.cppm",
]

LIBTHR_NEW = [
    "libthr/pbsd.userland.libthr.cond_timed.cppm",
    "libthr/pbsd.userland.libthr.rwlock_timed.cppm",
]

USR_B12 = [
    "biff", "beep", "morse", "killall", "enigma", "ipcrm", "ipcs", "getent",
    "showmount", "finger", "vacation", "unifdef", "at", "ministat",
]


def write_modules() -> None:
    for rel, body in MODULES.items():
        path = UL / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body, encoding="utf-8")
        print(f"  wrote {path.relative_to(ROOT)}")


def patch_file(path: Path, needle: str, insert: str, *, after: bool = True) -> None:
    text = path.read_text(encoding="utf-8")
    if insert.strip() in text:
        return
    if needle not in text:
        raise SystemExit(f"needle not found in {path}: {needle!r}")
    if after:
        text = text.replace(needle, needle + insert, 1)
    else:
        text = text.replace(needle, insert + needle, 1)
    path.write_text(text, encoding="utf-8")


def patch_cmake() -> None:
    text = CMAKE.read_text(encoding="utf-8")

    libc_needle = "    libc/pbsd.userland.libc.string.strsignal.cppm\n    libc/pbsd.userland.libc.cppm"
    libc_add = (
        "    libc/pbsd.userland.libc.gen.fmtcheck.cppm\n"
        "    libc/pbsd.userland.libc.gen.parsetime.cppm\n"
        "    libc/pbsd.userland.libc.gen.getpeereid.cppm\n"
    )
    if "gen.fmtcheck.cppm" not in text:
        text = text.replace(libc_needle, libc_add + "    libc/pbsd.userland.libc.cppm", 1)

    msun_needle = "    msun/pbsd.userland.msun.nearbyint.cppm\n    msun/pbsd.userland.msun.cppm"
    msun_add = (
        "    msun/pbsd.userland.msun.erf.cppm\n"
        "    msun/pbsd.userland.msun.erfc.cppm\n"
        "    msun/pbsd.userland.msun.lgamma.cppm\n"
        "    msun/pbsd.userland.msun.remquo.cppm\n"
    )
    if "msun.erf.cppm" not in text:
        text = text.replace(msun_needle, msun_add + "    msun/pbsd.userland.msun.cppm", 1)

    libthr_needle = (
        "    libthr/pbsd.userland.libthr.mutex.timed.cppm\n    libthr/pbsd.userland.libthr.cppm"
    )
    libthr_add = (
        "    libthr/pbsd.userland.libthr.cond_timed.cppm\n"
        "    libthr/pbsd.userland.libthr.rwlock_timed.cppm\n"
    )
    if "cond_timed.cppm" not in text:
        text = text.replace(libthr_needle, libthr_add + "    libthr/pbsd.userland.libthr.cppm", 1)

    b12_block = """
# --- burst 12: libc/msun/libthr + usr.bin gaps ------------------------------
foreach(_ul_b12 IN ITEMS biff beep morse killall enigma ipcrm ipcs getent showmount finger vacation unifdef at ministat)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b12} usr.bin/pbsd.userland.${_ul_b12}.cppm)
endforeach()

"""
    if "_ul_b12" not in text:
        text = text.replace(
            "if(NOT TARGET pbsd_userland_bin)",
            b12_block + "if(NOT TARGET pbsd_userland_bin)",
            1,
        )

    iface_needle = "    pbsd_userland_tset)"
    iface_add = "\n".join(f"    pbsd_userland_{t}" for t in USR_B12) + "\n"
    if "pbsd_userland_biff" not in text:
        text = text.replace(iface_needle, iface_add + iface_needle, 1)

    CMAKE.write_text(text, encoding="utf-8")
    print("  patched CMakeLists.txt")


def patch_aggregates() -> None:
    libc = UL / "libc" / "pbsd.userland.libc.cppm"
    text = libc.read_text(encoding="utf-8")
    for imp in [
        "export import pbsd.userland.libc.gen.fmtcheck;",
        "export import pbsd.userland.libc.gen.parsetime;",
        "export import pbsd.userland.libc.gen.getpeereid;",
    ]:
        if imp not in text:
            text = text.replace(
                "export import pbsd.userland.libc.string.strsignal;",
                "export import pbsd.userland.libc.string.strsignal;\n" + imp,
                1,
            )
    libc.write_text(text, encoding="utf-8")

    msun = UL / "msun" / "pbsd.userland.msun.cppm"
    text = msun.read_text(encoding="utf-8")
    for imp in [
        "export import pbsd.userland.msun.erf;",
        "export import pbsd.userland.msun.erfc;",
        "export import pbsd.userland.msun.lgamma;",
        "export import pbsd.userland.msun.remquo;",
    ]:
        if imp not in text:
            text = text.replace(
                "export import pbsd.userland.msun.fmod;",
                "export import pbsd.userland.msun.fmod;\n" + imp,
                1,
            )
    msun.write_text(text, encoding="utf-8")

    libthr = UL / "libthr" / "pbsd.userland.libthr.cppm"
    text = libthr.read_text(encoding="utf-8")
    for imp in [
        "export import pbsd.userland.libthr.cond_timed;",
        "export import pbsd.userland.libthr.rwlock_timed;",
    ]:
        if imp not in text:
            text = text.replace(
                "export import pbsd.userland.libthr.mutex.timed;",
                "export import pbsd.userland.libthr.mutex.timed;\n" + imp,
                1,
            )
    libthr.write_text(text, encoding="utf-8")
    print("  patched aggregate .cppm re-exports")


def main() -> None:
    print(f"Burst 12: writing {len(MODULES)} hand-port modules …")
    write_modules()
    patch_cmake()
    patch_aggregates()
    total = len(MODULES) + 1  # getpeereid pre-existed
    print(f"Done — {total} burst-12 userland modules (incl. wired getpeereid).")


if __name__ == "__main__":
    main()
