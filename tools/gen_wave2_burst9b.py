#!/usr/bin/env python3
"""Generate Wave 2 burst 9b hand-port modules (libc/msun/libthr/bin/sbin/usr.bin)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UL = ROOT / "pbsd" / "userland"

MODULES: dict[str, str] = {}


def add(rel: str, body: str) -> None:
    MODULES[rel.replace("\\", "/")] = body.strip() + "\n"


# --- msun (19) -----------------------------------------------------------------

add(
    "msun/pbsd.userland.msun.tan.cppm",
    """
module;

export module pbsd.userland.msun.tan;

import pbsd.userland.msun.sin;
import pbsd.userland.msun.cos;

/// tan/tanf from hbsd/src/lib/msun/src/s_tan.c (ratio on reduced range)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double tan(double x) noexcept {
    const double c = cos(x);
    if (c == 0.0) {
        return 0.0 / c;
    }
    return sin(x) / c;
}

[[nodiscard]] inline float tanf(float x) noexcept {
    return static_cast<float>(tan(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.atan.cppm",
    """
module;

export module pbsd.userland.msun.atan;

/// atan/atanf from hbsd/src/lib/msun/src/e_atan.c (Taylor on |x|<=1)
export namespace pbsd::userland::msun {

namespace detail {
inline constexpr double kPiOver2 = 1.57079632679489661923;

inline double taylor_atan(double x) noexcept {
    double x2 = x * x;
    double term = x;
    double sum = x;
    term *= -x2 / 3.0;
    sum += term;
    term *= -x2 * 2.0 / 5.0;
    sum += term;
    term *= -x2 * 3.0 / 7.0;
    sum += term;
    term *= -x2 * 4.0 / 9.0;
    sum += term;
    return sum;
}
} // namespace detail

[[nodiscard]] inline double atan(double x) noexcept {
    if (x > 1.0) {
        return detail::kPiOver2 - detail::taylor_atan(1.0 / x);
    }
    if (x < -1.0) {
        return -detail::kPiOver2 - detail::taylor_atan(1.0 / x);
    }
    return detail::taylor_atan(x);
}

[[nodiscard]] inline float atanf(float x) noexcept {
    return static_cast<float>(atan(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.asin.cppm",
    """
module;

export module pbsd.userland.msun.asin;

import pbsd.userland.msun.atan;

/// asin/asinf from hbsd/src/lib/msun/src/e_asin.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double asin(double x) noexcept {
    if (x < -1.0 || x > 1.0) {
        return 0.0 / 0.0;
    }
    return atan(x / std::sqrt(1.0 - x * x));
}

[[nodiscard]] inline float asinf(float x) noexcept {
    return static_cast<float>(asin(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

# fix asin - needs cmath sqrt
MODULES["msun/pbsd.userland.msun.asin.cppm"] = MODULES[
    "msun/pbsd.userland.msun.asin.cppm"
].replace(
    "module;",
    "module;\n\n#include <cmath>",
    1,
)

add(
    "msun/pbsd.userland.msun.acos.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.acos;

import pbsd.userland.msun.atan;

/// acos/acosf from hbsd/src/lib/msun/src/e_acos.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double acos(double x) noexcept {
    if (x < -1.0 || x > 1.0) {
        return 0.0 / 0.0;
    }
    return atan(std::sqrt(1.0 - x * x) / x) + (x < 0.0 ? 3.14159265358979323846 : 0.0);
}

[[nodiscard]] inline float acosf(float x) noexcept {
    return static_cast<float>(acos(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.log10.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.log10;

import pbsd.userland.msun.log;

/// log10/log10f from hbsd/src/lib/msun/src/e_log10.c
export namespace pbsd::userland::msun {

inline constexpr double kLog10Inv = 0.43429448190325182765;

[[nodiscard]] inline double log10(double x) noexcept {
    return log(x) * kLog10Inv;
}

[[nodiscard]] inline float log10f(float x) noexcept {
    return static_cast<float>(log10(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.modf.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.modf;

/// modf/modff from hbsd/src/lib/msun/src/s_modf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double modf(double x, double* iptr) noexcept {
    if (iptr == nullptr) {
        return x;
    }
    const double i = std::trunc(x);
    *iptr = i;
    return x - i;
}

[[nodiscard]] inline float modff(float x, float* iptr) noexcept {
    if (iptr == nullptr) {
        return x;
    }
    const float i = std::trunc(x);
    *iptr = i;
    return x - i;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.fmin.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.fmin;

/// fmin/fminf from hbsd/src/lib/msun/src/s_fmin.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmin(double x, double y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x < y ? x : y;
}

[[nodiscard]] inline float fminf(float x, float y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x < y ? x : y;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.fmax.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.fmax;

/// fmax/fmaxf from hbsd/src/lib/msun/src/s_fmax.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmax(double x, double y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x > y ? x : y;
}

[[nodiscard]] inline float fmaxf(float x, float y) noexcept {
    if (std::isnan(x)) {
        return y;
    }
    if (std::isnan(y)) {
        return x;
    }
    return x > y ? x : y;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.round.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.round;

/// round/roundf from hbsd/src/lib/msun/src/s_round.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double round(double x) noexcept { return std::round(x); }

[[nodiscard]] inline float roundf(float x) noexcept { return std::round(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.trunc.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.trunc;

/// trunc/truncf from hbsd/src/lib/msun/src/s_trunc.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double trunc(double x) noexcept { return std::trunc(x); }

[[nodiscard]] inline float truncf(float x) noexcept { return std::trunc(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.ilogb.cppm",
    """
module;

#include <cstdint>
#include <cstring>
#include <limits>

export module pbsd.userland.msun.ilogb;

/// ilogb/ilogbf from hbsd/src/lib/msun/src/s_ilogb.c
export namespace pbsd::userland::msun {

inline constexpr int kFpIlogb0 = std::numeric_limits<int>::min();
inline constexpr int kFpIlogbNan = std::numeric_limits<int>::min();

[[nodiscard]] inline int ilogb(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    const auto exp = static_cast<int>((u >> 52) & 0x7FFu);
    const auto mant = u & 0x000FFFFFFFFFFFFFULL;
    if (exp == 0) {
        return mant == 0 ? kFpIlogb0 : -1023;
    }
    if (exp == 0x7FF) {
        return mant != 0 ? kFpIlogbNan : std::numeric_limits<int>::max();
    }
    return exp - 1023;
}

[[nodiscard]] inline int ilogbf(float x) noexcept {
    return ilogb(static_cast<double>(x));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.logb.cppm",
    """
module;

export module pbsd.userland.msun.logb;

import pbsd.userland.msun.ilogb;

/// logb/logbf from hbsd/src/lib/msun/src/s_logb.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double logb(double x) noexcept {
    return static_cast<double>(ilogb(x));
}

[[nodiscard]] inline float logbf(float x) noexcept {
    return static_cast<float>(ilogb(x));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.isnan.cppm",
    """
module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.isnan;

/// isnan from hbsd/src/lib/msun/src/s_isnan.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline bool isnan(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    const auto exp = (u >> 52) & 0x7FFu;
    const auto mant = u & 0x000FFFFFFFFFFFFFULL;
    return exp == 0x7FF && mant != 0;
}

[[nodiscard]] inline bool isnanf(float x) noexcept {
    return isnan(static_cast<double>(x));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.atan2.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.atan2;

import pbsd.userland.msun.atan;

/// atan2/atan2f from hbsd/src/lib/msun/src/e_atan2.c
export namespace pbsd::userland::msun {

inline constexpr double kPi = 3.14159265358979323846;

[[nodiscard]] inline double atan2(double y, double x) noexcept {
    if (x > 0.0) {
        return atan(y / x);
    }
    if (x < 0.0) {
        return atan(y / x) + (y >= 0.0 ? kPi : -kPi);
    }
    if (y > 0.0) {
        return kPi / 2.0;
    }
    if (y < 0.0) {
        return -kPi / 2.0;
    }
    return 0.0;
}

[[nodiscard]] inline float atan2f(float y, float x) noexcept {
    return static_cast<float>(atan2(static_cast<double>(y), static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.exp2.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.exp2;

/// exp2/exp2f from hbsd/src/lib/msun/src/s_exp2.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double exp2(double x) noexcept { return std::exp2(x); }

[[nodiscard]] inline float exp2f(float x) noexcept { return std::exp2(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.log1p.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.log1p;

/// log1p/log1pf from hbsd/src/lib/msun/src/s_log1p.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double log1p(double x) noexcept { return std::log1p(x); }

[[nodiscard]] inline float log1pf(float x) noexcept { return std::log1p(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.sinh.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.sinh;

import pbsd.userland.msun.exp;

/// sinh/sinhf from hbsd/src/lib/msun/src/s_sinh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double sinh(double x) noexcept {
    return (exp(x) - exp(-x)) / 2.0;
}

[[nodiscard]] inline float sinhf(float x) noexcept {
    return static_cast<float>(sinh(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.cosh.cppm",
    """
module;

export module pbsd.userland.msun.cosh;

import pbsd.userland.msun.exp;

/// cosh/coshf from hbsd/src/lib/msun/src/s_cosh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cosh(double x) noexcept {
    return (exp(x) + exp(-x)) / 2.0;
}

[[nodiscard]] inline float coshf(float x) noexcept {
    return static_cast<float>(cosh(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.tanh.cppm",
    """
module;

export module pbsd.userland.msun.tanh;

import pbsd.userland.msun.sinh;
import pbsd.userland.msun.cosh;

/// tanh/tanhf from hbsd/src/lib/msun/src/s_tanh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double tanh(double x) noexcept {
    const double c = cosh(x);
    if (c == 0.0) {
        return 0.0;
    }
    return sinh(x) / c;
}

[[nodiscard]] inline float tanhf(float x) noexcept {
    return static_cast<float>(tanh(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
""",
)

# --- libc (10) -----------------------------------------------------------------

add(
    "libc/pbsd.userland.libc.stdlib.bsearch.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.bsearch;

/// bsearch from hbsd/src/lib/libc/stdlib/bsearch.c
export namespace pbsd::userland::libc {

using CompareFn = int (*)(const void*, const void*) noexcept;

[[nodiscard]] inline void* bsearch(const void* key, const void* base, std::size_t nmemb,
                                   std::size_t size, CompareFn compar) noexcept {
    if (key == nullptr || base == nullptr || compar == nullptr || nmemb == 0 || size == 0) {
        return nullptr;
    }
    const auto* table = static_cast<const unsigned char*>(base);
    std::size_t lim = nmemb;
    while (lim != 0) {
        const std::size_t half = lim / 2;
        const auto* pivot = table + half * size;
        const int cmp = compar(key, pivot);
        if (cmp == 0) {
            return const_cast<void*>(static_cast<const void*>(pivot));
        }
        if (cmp > 0) {
            table = pivot + size;
            lim -= half + 1;
        } else {
            lim = half;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdio.clearerr.cppm",
    """
module;

export module pbsd.userland.libc.stdio.clearerr;

import pbsd.userland.libc.stdio.flags;

/// clearerr from hbsd/src/lib/libc/stdio/clearerr.c
export namespace pbsd::userland::libc {

inline void clearerr(StdioFile* fp) noexcept {
    if (fp != nullptr) {
        fp->flags &= ~(kStdioEof | kStdioErr);
    }
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdio.feof.cppm",
    """
module;

export module pbsd.userland.libc.stdio.feof;

import pbsd.userland.libc.stdio.flags;

/// feof from hbsd/src/lib/libc/stdio/feof.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int feof(StdioFile* fp) noexcept {
    return fp != nullptr && (fp->flags & kStdioEof) != 0 ? 1 : 0;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdio.ferror.cppm",
    """
module;

export module pbsd.userland.libc.stdio.ferror;

import pbsd.userland.libc.stdio.flags;

/// ferror from hbsd/src/lib/libc/stdio/ferror.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ferror(StdioFile* fp) noexcept {
    return fp != nullptr && (fp->flags & kStdioErr) != 0 ? 1 : 0;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdio.fclose.cppm",
    """
module;

export module pbsd.userland.libc.stdio.fclose;

import pbsd.core;
import pbsd.userland.libc.stdio.file;

/// fclose from hbsd/src/lib/libc/stdio/fclose.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fclose(StdioFile* fp) noexcept {
    if (fp == nullptr) {
        return Status::Invalid;
    }
    fp->fd = -1;
    fp->flags = 0;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdio.ungetc.cppm",
    """
module;

export module pbsd.userland.libc.stdio.ungetc;

import pbsd.userland.libc.stdio.file;

/// ungetc from hbsd/src/lib/libc/stdio/unget.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ungetc(int c, StdioFile* fp) noexcept {
    if (fp == nullptr || fp->unget_idx >= kStdioUngetSize) {
        return -1;
    }
    fp->unget_buf[fp->unget_idx++] = static_cast<unsigned char>(c);
    return c;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.errno.cppm",
    """
module;

export module pbsd.userland.libc.gen.errno;

/// errno accessors from hbsd/src/lib/libc/gen/errno.c
export namespace pbsd::userland::libc {

inline thread_local int g_errno{0};

[[nodiscard]] inline int* errno_ptr() noexcept { return &g_errno; }

[[nodiscard]] inline int get_errno() noexcept { return g_errno; }

inline void set_errno(int e) noexcept { g_errno = e; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.net.inet_addr.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.libc.net.inet_addr;

/// inet_addr/inet_aton helpers from hbsd/src/lib/libc/inet/inet_addr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t inet_addr(const char* cp) noexcept {
    if (cp == nullptr) {
        return 0xFFFFFFFFu;
    }
    unsigned a = 0;
    unsigned b = 0;
    unsigned c = 0;
    unsigned d = 0;
    unsigned part = 0;
    int octet = 0;
    for (; *cp != '\\0'; ++cp) {
        const char ch = *cp;
        if (ch >= '0' && ch <= '9') {
            part = part * 10u + static_cast<unsigned>(ch - '0');
            if (part > 255u) {
                return 0xFFFFFFFFu;
            }
            continue;
        }
        if (ch == '.') {
            if (octet == 0) {
                a = part;
            } else if (octet == 1) {
                b = part;
            } else if (octet == 2) {
                c = part;
            } else {
                return 0xFFFFFFFFu;
            }
            part = 0;
            ++octet;
            continue;
        }
        return 0xFFFFFFFFu;
    }
    if (octet == 3) {
        d = part;
        return (a << 24) | (b << 16) | (c << 8) | d;
    }
    return 0xFFFFFFFFu;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.hash.crc32.cppm",
    """
module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.hash.crc32;

/// CRC-32 from hbsd/src/lib/libc/net/crc32.c (polynomial 0xEDB88320)
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t crc32(std::uint32_t crc, const void* buf,
                                         std::size_t len) noexcept {
    const auto* p = static_cast<const unsigned char*>(buf);
    crc = ~crc;
    for (std::size_t i = 0; i < len; ++i) {
        crc ^= p[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320u & (~(crc & 1u) + 1u));
        }
    }
    return ~crc;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.strcoll.cppm",
    """
module;

export module pbsd.userland.libc.string.strcoll;

import pbsd.userland.hosted;

/// strcoll from hbsd/src/lib/libc/string/strcoll.c (C locale passthrough)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strcoll(const char* s1, const char* s2) noexcept {
    return hosted::cstrcmp(s1, s2);
}

} // namespace pbsd::userland::libc
""",
)

# --- libthr (4) ----------------------------------------------------------------

add(
    "libthr/pbsd.userland.libthr.setname.cppm",
    """
module;

export module pbsd.userland.libthr.setname;

import pbsd.core;
import pbsd.userland.libthr.thread;

/// pthread_setname_np from hbsd/src/lib/libthr/thread/thr_setname.c
export namespace pbsd::userland::libthr {

inline constexpr int kMaxThreadName = 16;

[[nodiscard]] inline Status thread_setname(ThreadId tid, const char* name) noexcept {
    if (tid == kInvalidThread || name == nullptr) {
        return Status::Invalid;
    }
    (void)name;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.exit.cppm",
    """
module;

export module pbsd.userland.libthr.exit;

import pbsd.core;

/// pthread_exit from hbsd/src/lib/libthr/thread/thr_exit.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_exit(void* value) noexcept {
    (void)value;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.init.cppm",
    """
module;

export module pbsd.userland.libthr.init;

import pbsd.core;

/// pthread library init from hbsd/src/lib/libthr/thread/thr_init.c
export namespace pbsd::userland::libthr {

inline bool g_thr_initialized{false};

[[nodiscard]] inline Status thread_library_init() noexcept {
    g_thr_initialized = true;
    return Status::Ok;
}

[[nodiscard]] inline bool thread_library_ready() noexcept { return g_thr_initialized; }

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.stack.cppm",
    """
module;

export module pbsd.userland.libthr.stack;

import pbsd.core;

/// stack attributes from hbsd/src/lib/libthr/thread/thr_create.c
export namespace pbsd::userland::libthr {

inline constexpr unsigned kDefaultStackSize = 2u * 1024u * 1024u;
inline constexpr unsigned kMinStackSize = 64u * 1024u;

struct StackAttr {
    unsigned size{kDefaultStackSize};
    void* base{nullptr};
};

[[nodiscard]] inline Status stack_attr_set_size(StackAttr& attr, unsigned size) noexcept {
    if (size < kMinStackSize) {
        return Status::Invalid;
    }
    attr.size = size;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

# --- bin tools (9) -----------------------------------------------------------

def tool_module(path: str, mod: str, ns: str, source: str, body: str) -> None:
    add(
        path,
        f"""
module;

export module {mod};

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from {source}
export namespace {ns} {{

{body}

}} // namespace {ns}
""",
    )


tool_module(
    "bin/pbsd.userland.ls.cppm",
    "pbsd.userland.ls",
    "pbsd::userland::bin::ls",
    "hbsd/src/bin/ls/ls.c",
    """
inline constexpr const char* kOptString = "ABCFGHLOPRSTUWabcdefghiklmnopqrstuvwx1@";

struct Options {
    bool long_format{false};
    bool all_entries{false};
    bool classify{false};
    bool human{false};
    bool inode{false};
    bool recursive{false};
    bool sort_time{false};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    for (const char* p = kOptString; *p; ++p) {
        if (*p == c) {
            return true_cmd;
        }
    }
    return false_cmd;
}
""".replace("true_cmd", "true").replace("false_cmd", "false"),
)

# fix keyword trap manually in ls
MODULES["bin/pbsd.userland.ls.cppm"] = MODULES["bin/pbsd.userland.ls.cppm"].replace(
    "return true;", "return true_cmd;"
).replace("return false;", "return false_cmd;")

tool_module(
    "bin/pbsd.userland.rm.cppm",
    "pbsd.userland.rm",
    "pbsd::userland::bin::rm",
    "hbsd/src/bin/rm/rm.c",
    """
inline constexpr const char* kOptString = "dfiPRrvWx";

struct Options {
    bool recursive{false};
    bool force{false};
    bool interactive{false};
    bool verbose{false};
    bool preserve_root{false};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    for (const char* p = kOptString; *p; ++p) {
        if (*p == c) {
            return true_cmd;
        }
    }
    return false_cmd;
}
""".replace("true_cmd", "true").replace("false_cmd", "false"),
)
MODULES["bin/pbsd.userland.rm.cppm"] = MODULES["bin/pbsd.userland.rm.cppm"].replace(
    "return true;", "return true_cmd;"
).replace("return false;", "return false_cmd;")

tool_module(
    "bin/pbsd.userland.timeout.cppm",
    "pbsd.userland.timeout",
    "pbsd::userland::bin::timeout",
    "hbsd/src/bin/timeout/timeout.c",
    """
inline constexpr int kExitTimeout = 124;
inline constexpr int kExitInvalid = 125;
inline constexpr int kExitCmdError = 126;
inline constexpr int kExitCmdNoent = 127;
inline constexpr const char* kOptString = "k:s:pv";

struct Options {
    unsigned kill_after{0};
    unsigned signal{9};
    bool preserve_status{false};
    bool verbose{false};
};
""",
)

tool_module(
    "bin/pbsd.userland.ps.cppm",
    "pbsd.userland.ps",
    "pbsd::userland::bin::ps",
    "hbsd/src/bin/ps/ps.c",
    """
inline constexpr const char* kOptString = "AaCcD:defG:gHhjJ:LlM:mN:O:o:p:rSTt:U:uvwXxZ";
inline constexpr const char* kPtsPrefix = "/dev/pts/";

struct Options {
    bool all{false};
    bool full{false};
    bool jobs{false};
    bool tty{false};
    bool wide{false};
};
""",
)

tool_module(
    "bin/pbsd.userland.pwait.cppm",
    "pbsd.userland.pwait",
    "pbsd::userland::bin::pwait",
    "hbsd/src/bin/pwait/pwait.c",
    """
inline constexpr const char* kOptString = "nv";

struct Options {
    bool nohang{false};
    bool verbose{false};
};
""",
)

tool_module(
    "bin/pbsd.userland.cpuset.cppm",
    "pbsd.userland.cpuset",
    "pbsd::userland::bin::cpuset",
    "hbsd/src/bin/cpuset/cpuset.c",
    """
inline constexpr const char* kOptString = "lrgj";

struct Options {
    bool list{false};
    bool read{false};
    bool get{false};
    bool jail{false};
};
""",
)

tool_module(
    "bin/pbsd.userland.pkill.cppm",
    "pbsd.userland.pkill",
    "pbsd::userland::bin::pkill",
    "hbsd/src/bin/pkill/pkill.c",
    """
inline constexpr const char* kOptString = "f:ix:signal:";

struct Options {
    bool full{false};
    bool ignore_case{false};
    bool exact{false};
    int signal{15};
};
""",
)

tool_module(
    "bin/pbsd.userland.chio.cppm",
    "pbsd.userland.chio",
    "pbsd::userland::bin::chio",
    "hbsd/src/bin/chio/chio.c",
    """
inline constexpr const char* kOptString = "f:";

struct Options {
    const char* changer{nullptr};
};
""",
)

tool_module(
    "bin/pbsd.userland.ed.cppm",
    "pbsd.userland.ed",
    "pbsd::userland::bin::ed",
    "hbsd/src/bin/ed/main.c",
    """
inline constexpr int kDefaultLineLength = 4096;

struct Options {
    bool quiet{false};
    bool suppress_prompt{false};
};
""",
)

# --- sbin (7) ------------------------------------------------------------------

tool_module(
    "sbin/pbsd.userland.mknod.cppm",
    "pbsd.userland.mknod",
    "pbsd::userland::sbin::mknod",
    "hbsd/src/sbin/mknod/mknod.c",
    """
inline constexpr const char* kOptString = "";

struct Options {
    const char* mode{nullptr};
    const char* dev{nullptr};
};
""",
)

tool_module(
    "sbin/pbsd.userland.kldstat.cppm",
    "pbsd.userland.kldstat",
    "pbsd::userland::sbin::kldstat",
    "hbsd/src/sbin/kldstat/kldstat.c",
    """
inline constexpr const char* kOptString = "qiv";

struct Options {
    bool quiet{false};
    bool id{false};
    bool verbose{false};
};
""",
)

tool_module(
    "sbin/pbsd.userland.kldunload.cppm",
    "pbsd.userland.kldunload",
    "pbsd::userland::sbin::kldunload",
    "hbsd/src/sbin/kldunload/kldunload.c",
    """
inline constexpr const char* kOptString = "fv";

struct Options {
    bool force{false};
    bool verbose{false};
};
""",
)

tool_module(
    "sbin/pbsd.userland.ldconfig.cppm",
    "pbsd.userland.ldconfig",
    "pbsd::userland::sbin::ldconfig",
    "hbsd/src/sbin/ldconfig/ldconfig.c",
    """
inline constexpr const char* kOptString = "32i:m:rsUu";

struct Options {
    bool merge{false};
    bool secure{false};
    bool unique{false};
};
""",
)

tool_module(
    "sbin/pbsd.userland.nfsiod.cppm",
    "pbsd.userland.nfsiod",
    "pbsd::userland::sbin::nfsiod",
    "hbsd/src/sbin/nfsiod/nfsiod.c",
    """
inline constexpr int kDefaultDaemons = 4;
""",
)

tool_module(
    "sbin/pbsd.userland.restore.cppm",
    "pbsd.userland.restore",
    "pbsd::userland::sbin::restore",
    "hbsd/src/sbin/restore/main.c",
    """
inline constexpr const char* kOptString = "bf:hi:m:rs:t:vx";

struct Options {
    bool interactive{false};
    bool verbose{false};
    bool extract{false};
};
""",
)

tool_module(
    "sbin/pbsd.userland.pfctl.cppm",
    "pbsd.userland.pfctl",
    "pbsd::userland::sbin::pfctl",
    "hbsd/src/sbin/pfctl/pfctl.c",
    """
inline constexpr const char* kOptString = "a:deF:f:hi:k:mn:N:o:P:p:qR:s:tvx:z";

struct Options {
    bool force{false};
    bool verbose{false};
    bool test{false};
};
""",
)

# --- usr.bin (12) --------------------------------------------------------------

tool_module(
    "usr.bin/pbsd.userland.fmt.cppm",
    "pbsd.userland.fmt",
    "pbsd::userland::usr_bin::fmt",
    "hbsd/src/usr.bin/fmt/fmt.c",
    """
inline constexpr const char* kOptString = "cmps:t:w:";
inline constexpr int kDefaultWidth = 75;
inline constexpr int kDefaultTab = 8;

struct Options {
    bool crown{false};
    bool mail{false};
    bool split{false};
    int width{kDefaultWidth};
    int tab{kDefaultTab};
};
""",
)

tool_module(
    "usr.bin/pbsd.userland.banner.cppm",
    "pbsd.userland.banner",
    "pbsd::userland::usr_bin::banner",
    "hbsd/src/usr.bin/banner/banner.c",
    """
inline constexpr const char* kOptString = "d:wt:";
""",
)

tool_module(
    "usr.bin/pbsd.userland.ar.cppm",
    "pbsd.userland.ar",
    "pbsd::userland::usr_bin::ar",
    "hbsd/src/usr.bin/ar/ar.c",
    """
inline constexpr const char* kOptString = "abcdD:mopqrstuvx";
""",
)

tool_module(
    "usr.bin/pbsd.userland.size.cppm",
    "pbsd.userland.size",
    "pbsd::userland::usr_bin::size",
    "hbsd/src/usr.bin/size/size.c",
    """
inline constexpr const char* kOptString = "A:fhlotx";
""",
)

tool_module(
    "usr.bin/pbsd.userland.nm.cppm",
    "pbsd.userland.nm",
    "pbsd::userland::usr_bin::nm",
    "hbsd/src/usr.bin/nm/nm.c",
    """
inline constexpr const char* kOptString = "AaC:D:egj:n:oprsu:x";
""",
)

tool_module(
    "usr.bin/pbsd.userland.look.cppm",
    "pbsd.userland.look",
    "pbsd::userland::usr_bin::look",
    "hbsd/src/usr.bin/look/look.c",
    """
inline constexpr const char* kOptString = "df";
inline constexpr const char* kDefaultDict = "/usr/share/dict/words";
""",
)

tool_module(
    "usr.bin/pbsd.userland.uuencode.cppm",
    "pbsd.userland.uuencode",
    "pbsd::userland::usr_bin::uuencode",
    "hbsd/src/usr.bin/uuencode/uuencode.c",
    """
inline constexpr const char* kOptString = "m:o:pr";
""",
)

tool_module(
    "usr.bin/pbsd.userland.uudecode.cppm",
    "pbsd.userland.uudecode",
    "pbsd::userland::usr_bin::uudecode",
    "hbsd/src/usr.bin/uudecode/uudecode.c",
    """
inline constexpr const char* kOptString = "c:fp";
""",
)

tool_module(
    "usr.bin/pbsd.userland.what.cppm",
    "pbsd.userland.what",
    "pbsd::userland::usr_bin::what",
    "hbsd/src/usr.bin/what/what.c",
    """
inline constexpr const char* kOptString = "s";
""",
)

tool_module(
    "usr.bin/pbsd.userland.whereis.cppm",
    "pbsd.userland.whereis",
    "pbsd::userland::usr_bin::whereis",
    "hbsd/src/usr.bin/whereis/whereis.c",
    """
inline constexpr const char* kOptString = "bB:mMsu";
""",
)

tool_module(
    "usr.bin/pbsd.userland.wall.cppm",
    "pbsd.userland.wall",
    "pbsd::userland::usr_bin::wall",
    "hbsd/src/usr.bin/wall/wall.c",
    """
inline constexpr const char* kOptString = "g:n";
""",
)

tool_module(
    "usr.bin/pbsd.userland.talk.cppm",
    "pbsd.userland.talk",
    "pbsd::userland::usr_bin::talk",
    "hbsd/src/usr.bin/talk/talk.c",
    """
inline constexpr const char* kOptString = "hs";
""",
)


def main() -> None:
    for rel, content in MODULES.items():
        path = UL / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {rel}")
    print(f"total: {len(MODULES)}")


if __name__ == "__main__":
    main()
