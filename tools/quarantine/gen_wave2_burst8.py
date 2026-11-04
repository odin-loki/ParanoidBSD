#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 2 burst 8 hand-port modules (+40 libc, +15 msun, +10 libthr, +5 rtld)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UL = ROOT / "pbsd" / "userland"

MODULES: dict[str, str] = {}


def add(rel: str, body: str) -> None:
    MODULES[rel.replace("\\", "/")] = body.strip() + "\n"


# --- libc (30) ----------------------------------------------------------------

add(
    "libc/pbsd.userland.libc.stdbit.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.libc.stdbit;

/// stdc_count_* / stdc_has_single_bit from hbsd/src/lib/libc/stdbit/*.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline unsigned popcount_u32(unsigned x) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned>(__builtin_popcount(x));
#else
    unsigned n = 0;
    while (x) {
        n += x & 1u;
        x >>= 1;
    }
    return n;
#endif
}

[[nodiscard]] inline unsigned popcount_u64(std::uint64_t x) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned>(__builtin_popcountll(x));
#else
    unsigned n = 0;
    while (x) {
        n += static_cast<unsigned>(x & 1u);
        x >>= 1;
    }
    return n;
#endif
}
} // namespace detail

[[nodiscard]] inline unsigned stdc_count_ones_uc(unsigned char x) noexcept {
    return detail::popcount_u32(x);
}
[[nodiscard]] inline unsigned stdc_count_ones_us(unsigned short x) noexcept {
    return detail::popcount_u32(x);
}
[[nodiscard]] inline unsigned stdc_count_ones_ui(unsigned x) noexcept {
    return detail::popcount_u32(x);
}
[[nodiscard]] inline unsigned stdc_count_ones_ul(unsigned long x) noexcept {
    return detail::popcount_u64(static_cast<std::uint64_t>(x));
}
[[nodiscard]] inline unsigned stdc_count_ones_ull(unsigned long long x) noexcept {
    return detail::popcount_u64(x);
}

[[nodiscard]] inline unsigned stdc_count_zeros_uc(unsigned char x) noexcept {
    return static_cast<unsigned>(sizeof(unsigned char) * 8) - stdc_count_ones_uc(x);
}
[[nodiscard]] inline unsigned stdc_count_zeros_ui(unsigned x) noexcept {
    return static_cast<unsigned>(sizeof(unsigned) * 8) - stdc_count_ones_ui(x);
}

[[nodiscard]] inline bool stdc_has_single_bit_ui(unsigned x) noexcept {
    return x != 0 && (x & (x - 1u)) == 0u;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.bit.ext.cppm",
    """
module;

#include <climits>
#include <cstdint>

export module pbsd.userland.libc.string.bit.ext;

/// ffsll/flsll from hbsd/src/lib/libc/string/{ffsll,flsll}.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline int ctz_u64(std::uint64_t x) noexcept {
    if (x == 0) {
        return 64;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#else
    int n = 0;
    while ((x & 1u) == 0) {
        x >>= 1;
        ++n;
    }
    return n;
#endif
}

[[nodiscard]] inline int clz_u64(std::uint64_t x) noexcept {
    if (x == 0) {
        return 64;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clzll(x);
#else
    int n = 0;
    while (x <= 0x7FFFFFFFFFFFFFFFULL) {
        x <<= 1;
        ++n;
    }
    return n;
#endif
}
} // namespace detail

[[nodiscard]] inline int ffsll(long long mask) noexcept {
    return mask == 0 ? 0 : detail::ctz_u64(static_cast<std::uint64_t>(mask)) + 1;
}

[[nodiscard]] inline int flsll(long long mask) noexcept {
    return mask == 0 ? 0
                      : static_cast<int>(sizeof(long long) * CHAR_BIT) -
                            detail::clz_u64(static_cast<std::uint64_t>(mask));
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.copy.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.copy;

/// wcpcpy/wcpncpy from hbsd/src/lib/libc/string/{wcpcpy,wcpncpy}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wcpcpy(wchar_t* dst, const wchar_t* src) noexcept {
    while ((*dst = *src) != L'\\0') {
        ++dst;
        ++src;
    }
    return dst;
}

[[nodiscard]] inline wchar_t* wcpncpy(wchar_t* dst, const wchar_t* src, std::size_t n) noexcept {
    wchar_t* d = dst;
    while (n > 0 && *src != L'\\0') {
        *d++ = *src++;
        --n;
    }
    while (n > 0) {
        *d++ = L'\\0';
        --n;
    }
    return dst;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.move.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.move;

/// wmemmove/wmemcpy from hbsd/src/lib/libc/string/{wmemmove,wmemcpy}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wmemcpy(wchar_t* dst, const wchar_t* src, std::size_t n) noexcept {
    wchar_t* d = dst;
    while (n-- > 0) {
        *d++ = *src++;
    }
    return dst;
}

[[nodiscard]] inline wchar_t* wmemmove(wchar_t* dst, const wchar_t* src, std::size_t n) noexcept {
    if (dst <= src || dst >= src + n) {
        return wmemcpy(dst, src, n);
    }
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
    return dst;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.casecmp.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.casecmp;

/// wcscasecmp/wcsncasecmp from hbsd/src/lib/libc/string/{wcscasecmp,wcsncasecmp}.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline wchar_t towlower_ascii(wchar_t c) noexcept {
    return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c - L'A' + L'a') : c;
}
} // namespace detail

[[nodiscard]] inline int wcscasecmp(const wchar_t* a, const wchar_t* b) noexcept {
    while (*a != L'\\0' && *b != L'\\0') {
        const wchar_t ca = detail::towlower_ascii(*a);
        const wchar_t cb = detail::towlower_ascii(*b);
        if (ca != cb) {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        ++a;
        ++b;
    }
    return static_cast<int>(detail::towlower_ascii(*a)) -
           static_cast<int>(detail::towlower_ascii(*b));
}

[[nodiscard]] inline int wcsncasecmp(const wchar_t* a, const wchar_t* b, std::size_t n) noexcept {
    while (n > 0 && *a != L'\\0' && *b != L'\\0') {
        const wchar_t ca = detail::towlower_ascii(*a);
        const wchar_t cb = detail::towlower_ascii(*b);
        if (ca != cb) {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        ++a;
        ++b;
        --n;
    }
    if (n == 0) {
        return 0;
    }
    return static_cast<int>(detail::towlower_ascii(*a)) -
           static_cast<int>(detail::towlower_ascii(*b));
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.search.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.search;

/// wcslen/wcsrchr from hbsd/src/lib/libc/string/{wcslen,wcsrchr}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t wcslen(const wchar_t* s) noexcept {
    const wchar_t* p = s;
    while (*p != L'\\0') {
        ++p;
    }
    return static_cast<std::size_t>(p - s);
}

[[nodiscard]] inline const wchar_t* wcsrchr(const wchar_t* s, wchar_t c) noexcept {
    const wchar_t* last = nullptr;
    while (*s != L'\\0') {
        if (*s == c) {
            last = s;
        }
        ++s;
    }
    if (c == L'\\0') {
        return s;
    }
    return last;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdlib.insque.cppm",
    """
module;

export module pbsd.userland.libc.stdlib.insque;

/// insque/remque from hbsd/src/lib/libc/stdlib/{insque,remque}.c
export namespace pbsd::userland::libc {

struct QueElem {
    QueElem* prev{nullptr};
    QueElem* next{nullptr};
};

inline void insque(QueElem& elem, QueElem* pred) noexcept {
    if (pred == nullptr) {
        elem.prev = nullptr;
        elem.next = nullptr;
        return;
    }
    QueElem* next = pred->next;
    if (next != nullptr) {
        next->prev = &elem;
    }
    pred->next = &elem;
    elem.prev = pred;
    elem.next = next;
}

inline void remque(QueElem& elem) noexcept {
    if (elem.prev != nullptr) {
        elem.prev->next = elem.next;
    }
    if (elem.next != nullptr) {
        elem.next->prev = elem.prev;
    }
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdlib.tree.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.tree;

/// tfind/tsearch/twalk/tdelete concepts from hbsd/src/lib/libc/stdlib/tree.c
export namespace pbsd::userland::libc {

using TreeCompare = int (*)(const void*, const void*) noexcept;
using TreeAction = void (*)(const void*, void*) noexcept;

struct TreeNode {
    const void* key{nullptr};
    TreeNode* left{nullptr};
    TreeNode* right{nullptr};
};

[[nodiscard]] inline TreeNode* tfind(const void* key, TreeNode* const* rootp,
                                     TreeCompare compar) noexcept {
    TreeNode* node = *rootp;
    while (node != nullptr) {
        const int cmp = compar(key, node->key);
        if (cmp == 0) {
            return node;
        }
        node = cmp < 0 ? node->left : node->right;
    }
    return nullptr;
}

[[nodiscard]] inline TreeNode* tsearch(const void* key, TreeNode** rootp,
                                       TreeCompare compar) noexcept {
    if (*rootp == nullptr) {
        *rootp = new TreeNode{key, nullptr, nullptr};
        return *rootp;
    }
    TreeNode* node = *rootp;
    for (;;) {
        const int cmp = compar(key, node->key);
        if (cmp == 0) {
            return node;
        }
        TreeNode** slot = cmp < 0 ? &node->left : &node->right;
        if (*slot == nullptr) {
            *slot = new TreeNode{key, nullptr, nullptr};
            return *slot;
        }
        node = *slot;
    }
}

inline void twalk(const TreeNode* root, TreeAction action, void* cookie) noexcept {
    if (root == nullptr) {
        return;
    }
    twalk(root->left, action, cookie);
    action(root->key, cookie);
    twalk(root->right, action, cookie);
}

[[nodiscard]] inline TreeNode* tdelete(const void* key, TreeNode** rootp,
                                       TreeCompare compar) noexcept {
    TreeNode** parent_link = rootp;
    TreeNode* node = *rootp;
    while (node != nullptr) {
        const int cmp = compar(key, node->key);
        if (cmp == 0) {
            if (node->left == nullptr) {
                TreeNode* victim = node;
                *parent_link = node->right;
                delete victim;
                return *parent_link;
            }
            if (node->right == nullptr) {
                TreeNode* victim = node;
                *parent_link = node->left;
                delete victim;
                return *parent_link;
            }
            TreeNode* succ = node->right;
            while (succ->left != nullptr) {
                succ = succ->left;
            }
            const void* tmp = node->key;
            node->key = succ->key;
            return tdelete(tmp, &node->right, compar);
        }
        parent_link = cmp < 0 ? &node->left : &node->right;
        node = *parent_link;
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdlib.a64l.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.a64l;

/// a64l/l64a from hbsd/src/lib/libc/stdlib/{a64l,l64a}.c
export namespace pbsd::userland::libc {

inline constexpr int kADot = 46;
inline constexpr int kASlash = 47;
inline constexpr int kA0 = 48;
inline constexpr int kAA = 65;
inline constexpr int kAa = 97;

[[nodiscard]] inline long a64l(const char* s) noexcept {
    long value = 0;
    int shift = 0;
    for (int i = 0; s[i] != '\\0' && i < 6; ++i) {
        int digit;
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c <= static_cast<unsigned char>(kASlash)) {
            digit = static_cast<int>(c) - kASlash + 1;
        } else if (c <= static_cast<unsigned char>(kA0 + 9)) {
            digit = static_cast<int>(c) - kA0 + 2;
        } else if (c <= static_cast<unsigned char>(kAA + 25)) {
            digit = static_cast<int>(c) - kAA + 12;
        } else {
            digit = static_cast<int>(c) - kAa + 38;
        }
        value |= static_cast<long>(digit) << shift;
        shift += 6;
    }
    return value;
}

[[nodiscard]] inline char* l64a(long value, char* buf, std::size_t buflen) noexcept {
    static const char digits[] =
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::size_t i = 0;
    do {
        if (i + 1 >= buflen) {
            break;
        }
        buf[i++] = digits[value & 0x3f];
        value >>= 6;
    } while (value != 0 && i < 6);
    if (i < buflen) {
        buf[i] = '\\0';
    }
    return buf;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdlib.strtoq.cppm",
    """
module;

#include <cerrno>
#include <climits>

export module pbsd.userland.libc.stdlib.strtoq;

/// strtoq/strtouq from hbsd/src/lib/libc/stdlib/{strtoq,strtouq}.c (C-locale subset)
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtoq(const char* nptr, char** endptr, int base) noexcept {
    return strtoll(nptr, endptr, base);
}

[[nodiscard]] inline unsigned long long strtouq(const char* nptr, char** endptr,
                                                  int base) noexcept {
    return strtoull(nptr, endptr, base);
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdlib.reallocf.cppm",
    """
module;

#include <cstdlib>

export module pbsd.userland.libc.stdlib.reallocf;

/// reallocf concept from hbsd/src/lib/libc/stdlib/reallocf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* reallocf(void* ptr, std::size_t size) noexcept {
    void* np = std::realloc(ptr, size);
    if (np == nullptr && size != 0) {
        std::free(ptr);
    }
    return np;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.assert.cppm",
    """
module;

export module pbsd.userland.libc.gen.assert;

/// __assert hook from hbsd/src/lib/libc/gen/assert.c (logic-only; no stderr I/O)
export namespace pbsd::userland::libc {

using AssertHandler = void (*)(const char* func, const char* file, int line,
                               const char* failedexpr) noexcept;

inline AssertHandler g_assert_handler = nullptr;

inline void set_assert_handler(AssertHandler h) noexcept { g_assert_handler = h; }

[[nodiscard]] inline AssertHandler assert_handler() noexcept { return g_assert_handler; }

inline void report_assert_failure(const char* func, const char* file, int line,
                                  const char* failedexpr) noexcept {
    if (g_assert_handler != nullptr) {
        g_assert_handler(func, file, line, failedexpr);
    }
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.time.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.libc.gen.time;

/// time(3) epoch helpers from hbsd/src/lib/libc/gen/time.c (logic-only)
export namespace pbsd::userland::libc {

using TimeT = std::int64_t;

[[nodiscard]] inline TimeT time_now(TimeT* out) noexcept {
    const TimeT t = 0;
    if (out != nullptr) {
        *out = t;
    }
    return t;
}

[[nodiscard]] inline bool is_leap_year(int year) noexcept {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

[[nodiscard]] inline int days_in_month(int year, int month) noexcept {
    static constexpr int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 1 && is_leap_year(year)) {
        return 29;
    }
    return kDays[month];
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.uname.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.uname;

/// uname struct from hbsd/src/lib/libc/gen/uname.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kUnameSysnameLen = 256;
inline constexpr std::size_t kUnameNodenameLen = 256;
inline constexpr std::size_t kUnameReleaseLen = 256;
inline constexpr std::size_t kUnameVersionLen = 256;
inline constexpr std::size_t kUnameMachineLen = 256;

struct UtsName {
    char sysname[kUnameSysnameLen];
    char nodename[kUnameNodenameLen];
    char release[kUnameReleaseLen];
    char version[kUnameVersionLen];
    char machine[kUnameMachineLen];
};

inline void fill_default_uname(UtsName& out) noexcept {
    const char sys[] = "PBSD";
    const char rel[] = "0.1";
    const char ver[] = "PBSD Userland";
    const char mach[] = "generic";
    for (std::size_t i = 0; i < kUnameSysnameLen; ++i) {
        out.sysname[i] = i < sizeof(sys) - 1 ? sys[i] : '\\0';
    }
    for (std::size_t i = 0; i < kUnameReleaseLen; ++i) {
        out.release[i] = i < sizeof(rel) - 1 ? rel[i] : '\\0';
    }
    for (std::size_t i = 0; i < kUnameVersionLen; ++i) {
        out.version[i] = i < sizeof(ver) - 1 ? ver[i] : '\\0';
    }
    for (std::size_t i = 0; i < kUnameMachineLen; ++i) {
        out.machine[i] = i < sizeof(mach) - 1 ? mach[i] : '\\0';
    }
    out.nodename[0] = '\\0';
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.isatty.cppm",
    """
module;

export module pbsd.userland.libc.gen.isatty;

/// isatty/ttyslot concepts from hbsd/src/lib/libc/gen/{isatty,ttyslot}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool isatty_fd(int fd) noexcept {
    return fd >= 0 && fd <= 2;
}

[[nodiscard]] inline int ttyslot_from_fd(int fd) noexcept {
    return isatty_fd(fd) ? fd + 1 : 0;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.raise.cppm",
    """
module;

export module pbsd.userland.libc.gen.raise;

/// raise(3) signal dispatch concept from hbsd/src/lib/libc/gen/raise.c
export namespace pbsd::userland::libc {

using SignalHandler = void (*)(int) noexcept;

inline SignalHandler g_signal_handler = nullptr;

inline void set_signal_handler(SignalHandler h) noexcept { g_signal_handler = h; }

[[nodiscard]] inline int raise_signal(int sig) noexcept {
    if (g_signal_handler != nullptr) {
        g_signal_handler(sig);
        return 0;
    }
    return -1;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.arc4random.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.libc.gen.arc4random;

/// arc4random_uniform from hbsd/src/lib/libc/gen/arc4random_uniform.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t arc4random_uniform(std::uint32_t upper_bound,
                                                     std::uint32_t raw) noexcept {
    if (upper_bound < 2) {
        return 0;
    }
    const std::uint32_t min = static_cast<std::uint32_t>(-upper_bound) % upper_bound;
    std::uint32_t r = raw;
    while (r < min) {
        r = raw;
    }
    return r % upper_bound;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.hostname.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.hostname;

/// get/set hostname/domainname from hbsd/src/lib/libc/gen/{get,set}*name.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kHostNameMax = 256;

[[nodiscard]] inline bool copy_host_string(char* dst, std::size_t dstlen, const char* src) noexcept {
    if (dstlen == 0) {
        return false;
    }
    std::size_t i = 0;
    while (src[i] != '\\0' && i + 1 < dstlen) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\\0';
    return src[i] == '\\0';
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.opendir.cppm",
    """
module;

export module pbsd.userland.libc.gen.opendir;

/// opendir/closedir concepts from hbsd/src/lib/libc/gen/opendir.c
export namespace pbsd::userland::libc {

struct DirEntry {
    const char* name{nullptr};
    int fd{-1};
};

struct Dir {
    int fd{-1};
    unsigned index{0};
};

[[nodiscard]] inline bool dir_valid(const Dir& d) noexcept { return d.fd >= 0; }

inline void closedir(Dir& d) noexcept {
    d.fd = -1;
    d.index = 0;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.dirfd.cppm",
    """
module;

export module pbsd.userland.libc.gen.dirfd;

/// dirfd from hbsd/src/lib/libc/gen/dirfd.c
export namespace pbsd::userland::libc {

struct Dir {
    int fd{-1};
};

[[nodiscard]] inline int dirfd(const Dir& d) noexcept { return d.fd; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.getbootfile.cppm",
    """
module;

export module pbsd.userland.libc.gen.getbootfile;

/// getbootfile from hbsd/src/lib/libc/gen/getbootfile.c
export namespace pbsd::userland::libc {

inline constexpr const char* kDefaultBootfile = "/boot/kernel/kernel";

[[nodiscard]] inline const char* default_bootfile() noexcept { return kDefaultBootfile; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.utime.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.libc.gen.utime;

/// utime/utimes concepts from hbsd/src/lib/libc/gen/utime.c
export namespace pbsd::userland::libc {

struct Utimbuf {
    std::int64_t actime{0};
    std::int64_t modtime{0};
};

[[nodiscard]] inline bool utimbuf_valid(const Utimbuf& u) noexcept {
    return u.actime >= 0 && u.modtime >= 0;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.vis.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.vis;

/// unvis/vis subset from hbsd/src/lib/libc/gen/unvis-compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int vis_char(unsigned char c, char* dst, std::size_t dstlen) noexcept {
    if (dstlen < 2) {
        return -1;
    }
    if (c >= 0x20 && c < 0x7f && c != '\\\\' && c != '\\'') {
        dst[0] = static_cast<char>(c);
        dst[1] = '\\0';
        return 1;
    }
    if (dstlen < 5) {
        return -1;
    }
    dst[0] = '\\\\';
    dst[1] = static_cast<char>('0' + ((c >> 6) & 7));
    dst[2] = static_cast<char>('0' + ((c >> 3) & 7));
    dst[3] = static_cast<char>('0' + (c & 7));
    dst[4] = '\\0';
    return 4;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.uuid.cppm",
    """
module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.libc.uuid;

/// uuid_create/uuid_create_nil/uuid_hash from hbsd/src/lib/libc/uuid/*.c
export namespace pbsd::userland::libc {

struct Uuid {
    std::uint32_t time_low{0};
    std::uint16_t time_mid{0};
    std::uint16_t time_hi_and_version{0};
    std::uint8_t clock_seq_hi_and_reserved{0};
    std::uint8_t clock_seq_low{0};
    std::uint8_t node[6]{};
};

inline void uuid_create_nil(Uuid& u) noexcept { u = Uuid{}; }

[[nodiscard]] inline bool uuid_is_nil(const Uuid& u) noexcept {
    Uuid nil{};
    return std::memcmp(&u, &nil, sizeof(u)) == 0;
}

[[nodiscard]] inline std::uint32_t uuid_hash(const Uuid& u) noexcept {
    std::uint32_t h = u.time_low;
    h ^= static_cast<std::uint32_t>(u.time_mid) << 16;
    h ^= static_cast<std::uint32_t>(u.time_hi_and_version) << 8;
    h ^= static_cast<std::uint32_t>(u.clock_seq_hi_and_reserved) << 24;
    h ^= static_cast<std::uint32_t>(u.clock_seq_low);
    for (int i = 0; i < 6; ++i) {
        h = (h << 5) + h + u.node[i];
    }
    return h;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.net.sockatmark.cppm",
    """
module;

export module pbsd.userland.libc.net.sockatmark;

/// sockatmark from hbsd/src/lib/libc/net/sockatmark.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool sockatmark_oob_index(int oobmark) noexcept { return oobmark != 0; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.net.vars.cppm",
    """
module;

export module pbsd.userland.libc.net.vars;

/// h_errno / net vars from hbsd/src/lib/libc/net/vars.c
export namespace pbsd::userland::libc {

inline int g_h_errno = 0;

[[nodiscard]] inline int h_errno_value() noexcept { return g_h_errno; }

inline void set_h_errno(int e) noexcept { g_h_errno = e; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.hash.log2.cppm",
    """
module;

export module pbsd.userland.libc.hash.log2;

/// hash_log2 from hbsd/src/lib/libc/db/hash/hash_log2.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned hash_log2(unsigned num) noexcept {
    unsigned i = 0;
    unsigned n = num;
    while (n > 1) {
        n >>= 1;
        ++i;
    }
    return i;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gdtoa.ldis.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.libc.gdtoa.ldis;

/// machdep_ldisd helpers from hbsd/src/lib/libc/gdtoa/machdep_ldisd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int extract_exponent_bits(std::uint64_t bits) noexcept {
    return static_cast<int>((bits >> 52) & 0x7ff) - 1023;
}

[[nodiscard]] inline bool is_nan_bits(std::uint64_t bits) noexcept {
    return ((bits >> 52) & 0x7ff) == 0x7ff && (bits & 0xfffffffffffffULL) != 0;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.cap.cppm",
    """
module;

export module pbsd.userland.libc.gen.cap;

/// cap_sandboxed from hbsd/src/lib/libc/gen/cap_sandboxed.c
export namespace pbsd::userland::libc {

inline bool g_cap_sandboxed = false;

[[nodiscard]] inline bool cap_sandboxed() noexcept { return g_cap_sandboxed; }

inline void set_cap_sandboxed(bool v) noexcept { g_cap_sandboxed = v; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.getgrouplist.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.getgrouplist;

/// getgrouplist from hbsd/src/lib/libc/gen/getgrouplist.c (logic-only merge)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int merge_group_list(int* groups, int ngroups, int gid, int maxgroups) noexcept {
    if (maxgroups <= 0) {
        return -1;
    }
    for (int i = 0; i < ngroups; ++i) {
        if (groups[i] == gid) {
            return ngroups;
        }
    }
    if (ngroups >= maxgroups) {
        return -1;
    }
    groups[ngroups] = gid;
    return ngroups + 1;
}

} // namespace pbsd::userland::libc
""",
)

# --- msun (10) -----------------------------------------------------------------

add(
    "msun/pbsd.userland.msun.ceilf.cppm",
    """
module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ceilf;

/// ceilf from hbsd/src/lib/msun/src/s_ceilf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float ceilf(float x) noexcept {
    std::uint32_t i0;
    std::memcpy(&i0, &x, sizeof(i0));
    const int j0 = static_cast<int>(((i0 >> 23) & 0xffU) - 0x7fU);
    if (j0 < 23) {
        if (j0 < 0) {
            if ((i0 & 0x7fffffffU) != 0U) {
                i0 = (i0 & 0x80000000U) ? 0x80000000U : 0x3f800000U;
            }
        } else {
            const std::uint32_t i = 0x007fffffU >> j0;
            if ((i0 & i) == 0U) {
                return x;
            }
            if (i0 < 0x80000000U) {
                i0 += 0x00800000U >> j0;
            }
            i0 &= ~i;
        }
    } else if (j0 == 0x80) {
        return x + x;
    }
    std::memcpy(&x, &i0, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.floorf.cppm",
    """
module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.floorf;

/// floorf from hbsd/src/lib/msun/src/s_floorf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float floorf(float x) noexcept {
    std::uint32_t i0;
    std::memcpy(&i0, &x, sizeof(i0));
    const int j0 = static_cast<int>(((i0 >> 23) & 0xffU) - 0x7fU);
    if (j0 < 23) {
        if (j0 < 0) {
            if ((i0 & 0x7fffffffU) != 0U) {
                i0 = (i0 >= 0x80000000U) ? 0x80000000U : 0;
            }
        } else {
            const std::uint32_t i = 0x007fffffU >> j0;
            if ((i0 & i) == 0U) {
                return x;
            }
            if (i0 >= 0x80000000U) {
                i0 += 0x00800000U >> j0;
            }
            i0 &= ~i;
        }
    } else if (j0 == 0x80) {
        return x + x;
    }
    std::memcpy(&x, &i0, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.frexp.cppm",
    """
module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.frexp;

/// frexp/frexpf from hbsd/src/lib/msun/src/s_frexp.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double frexp(double x, int* exp) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    const int e = static_cast<int>((u >> 52) & 0x7ff) - 1022;
    if (((u >> 52) & 0x7ff) == 0) {
        if (exp != nullptr) {
            *exp = 0;
        }
        return x;
    }
    u &= 0x800fffffffffffffULL;
    u |= static_cast<std::uint64_t>(1022) << 52;
    std::memcpy(&x, &u, sizeof(u));
    if (exp != nullptr) {
        *exp = e;
    }
    return x;
}

[[nodiscard]] inline float frexpf(float x, int* exp) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    const int e = static_cast<int>((u >> 23) & 0xff) - 126;
    if (((u >> 23) & 0xff) == 0) {
        if (exp != nullptr) {
            *exp = 0;
        }
        return x;
    }
    u &= 0x807fffffU;
    u |= 126U << 23;
    std::memcpy(&x, &u, sizeof(x));
    if (exp != nullptr) {
        *exp = e;
    }
    return x;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.scalbn.cppm",
    """
module;

#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.scalbn;

/// scalbn/scalbnf from hbsd/src/lib/msun/src/s_scalbn.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double scalbn(double x, int n) noexcept {
    return std::ldexp(x, n);
}

[[nodiscard]] inline float scalbnf(float x, int n) noexcept {
    return std::ldexp(x, n);
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.truncf.cppm",
    """
module;

export module pbsd.userland.msun.truncf;

import pbsd.userland.msun.ceilf;
import pbsd.userland.msun.floorf;

/// truncf from hbsd/src/lib/msun/src/s_truncf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float truncf(float x) noexcept {
    return x >= 0.0f ? floorf(x) : ceilf(x);
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.roundf.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.roundf;

/// roundf from hbsd/src/lib/msun/src/s_roundf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float roundf(float x) noexcept { return std::round(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.rintf.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.rintf;

/// rintf from hbsd/src/lib/msun/src/s_rintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float rintf(float x) noexcept { return std::nearbyint(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.signbit.cppm",
    """
module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.signbit;

/// signbit from hbsd/src/lib/msun/src/s_signbit.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline bool signbit(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    return (u >> 63) != 0;
}

[[nodiscard]] inline bool signbitf(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    return (u >> 31) != 0;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.isfinite.cppm",
    """
module;

#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.isfinite;

/// isfinite from hbsd/src/lib/msun/src/s_isfinite.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline bool isfinite(double x) noexcept { return std::isfinite(x); }

[[nodiscard]] inline bool isfinitef(float x) noexcept { return std::isfinite(x); }

[[nodiscard]] inline bool isfinite_bits(std::uint64_t bits) noexcept {
    return ((bits >> 52) & 0x7ff) != 0x7ff;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.fdim.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.fdim;

/// fdim from hbsd/src/lib/msun/src/s_fdim.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fdim(double x, double y) noexcept { return std::fdim(x, y); }

[[nodiscard]] inline float fdimf(float x, float y) noexcept { return std::fdim(x, y); }

} // namespace pbsd::userland::msun
""",
)

# --- libthr (8) ----------------------------------------------------------------

add(
    "libthr/pbsd.userland.libthr.spinlock.cppm",
    """
module;

export module pbsd.userland.libthr.spinlock;

import pbsd.core;

/// pthread spinlock concepts from hbsd/src/lib/libthr/thread/thr_spinlock.c
export namespace pbsd::userland::libthr {

struct SpinLock {
    ThreadId owner{kInvalidThread};
    bool locked{false};
};

[[nodiscard]] inline Status spin_init(SpinLock& s) noexcept {
    s = SpinLock{};
    return Status::Ok;
}

[[nodiscard]] inline Status spin_lock(SpinLock& s, ThreadId self) noexcept {
    if (s.locked && s.owner != self) {
        return Status::Busy;
    }
    s.locked = true;
    s.owner = self;
    return Status::Ok;
}

[[nodiscard]] inline Status spin_unlock(SpinLock& s, ThreadId self) noexcept {
    if (!s.locked || s.owner != self) {
        return Status::Denied;
    }
    s.locked = false;
    s.owner = kInvalidThread;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.sem.cppm",
    """
module;

export module pbsd.userland.libthr.sem;

import pbsd.core;

/// sem concepts from hbsd/src/lib/libthr/thread/thr_sem.c
export namespace pbsd::userland::libthr {

struct Semaphore {
    unsigned value{0};
};

[[nodiscard]] inline Status sem_init(Semaphore& s, unsigned initial) noexcept {
    s.value = initial;
    return Status::Ok;
}

[[nodiscard]] inline Status sem_post(Semaphore& s) noexcept {
    ++s.value;
    return Status::Ok;
}

[[nodiscard]] inline Status sem_wait(Semaphore& s) noexcept {
    if (s.value == 0) {
        return Status::Busy;
    }
    --s.value;
    return Status::Ok;
}

[[nodiscard]] inline Status sem_trywait(Semaphore& s) noexcept { return sem_wait(s); }

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.detach.cppm",
    """
module;

export module pbsd.userland.libthr.detach;

import pbsd.core;
import pbsd.userland.libthr.thread;

/// pthread_detach from hbsd/src/lib/libthr/thread/thr_detach.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_detach(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.kill.cppm",
    """
module;

export module pbsd.userland.libthr.kill;

import pbsd.core;
import pbsd.userland.libthr.thread;

/// pthread_kill from hbsd/src/lib/libthr/thread/thr_kill.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_kill(ThreadId tid, int sig) noexcept {
    if (tid == kInvalidThread || sig < 0) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.barrier.cppm",
    """
module;

export module pbsd.userland.libthr.barrier;

import pbsd.core;

/// pthread_barrier from hbsd/src/lib/libthr/thread/thr_barrier.c
export namespace pbsd::userland::libthr {

struct Barrier {
    unsigned count{0};
    unsigned waiting{0};
    unsigned generation{0};
};

[[nodiscard]] inline Status barrier_init(Barrier& b, unsigned count) noexcept {
    if (count == 0) {
        return Status::Invalid;
    }
    b.count = count;
    b.waiting = 0;
    b.generation = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status barrier_wait(Barrier& b, unsigned& generation_out) noexcept {
    ++b.waiting;
    if (b.waiting < b.count) {
        generation_out = b.generation;
        return Status::Busy;
    }
    b.waiting = 0;
    ++b.generation;
    generation_out = b.generation;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.mutexattr.cppm",
    """
module;

export module pbsd.userland.libthr.mutexattr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_mutexattr from hbsd/src/lib/libthr/thread/thr_mutexattr.c
export namespace pbsd::userland::libthr {

struct MutexAttr {
    MutexType type{MutexType::Normal};
    bool pshared{false};
};

[[nodiscard]] inline Status mutexattr_init(MutexAttr& a) noexcept {
    a = MutexAttr{};
    return Status::Ok;
}

[[nodiscard]] inline Status mutexattr_settype(MutexAttr& a, MutexType t) noexcept {
    a.type = t;
    return Status::Ok;
}

[[nodiscard]] inline MutexType mutexattr_gettype(const MutexAttr& a) noexcept { return a.type; }

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.condattr.cppm",
    """
module;

export module pbsd.userland.libthr.condattr;

import pbsd.core;

/// pthread_condattr from hbsd/src/lib/libthr/thread/thr_condattr.c
export namespace pbsd::userland::libthr {

struct CondAttr {
    bool pshared{false};
    int clock_id{0};
};

[[nodiscard]] inline Status condattr_init(CondAttr& a) noexcept {
    a = CondAttr{};
    return Status::Ok;
}

[[nodiscard]] inline Status condattr_setpshared(CondAttr& a, bool pshared) noexcept {
    a.pshared = pshared;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.rwlock.cppm",
    """
module;

export module pbsd.userland.libthr.rwlock;

import pbsd.core;
import pbsd.userland.libthr.thread;

/// pthread_rwlock concepts from hbsd/src/lib/libthr/thread/thr_rwlock*.c
export namespace pbsd::userland::libthr {

struct RwLock {
    ThreadId writer{kInvalidThread};
    unsigned readers{0};
};

[[nodiscard]] inline Status rwlock_init(RwLock& l) noexcept {
    l = RwLock{};
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_rdlock(RwLock& l) noexcept {
    if (l.writer != kInvalidThread) {
        return Status::Busy;
    }
    ++l.readers;
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_wrlock(RwLock& l, ThreadId self) noexcept {
    if (l.writer != kInvalidThread || l.readers > 0) {
        return Status::Busy;
    }
    l.writer = self;
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_unlock(RwLock& l, ThreadId self) noexcept {
    if (l.writer == self) {
        l.writer = kInvalidThread;
        return Status::Ok;
    }
    if (l.readers > 0) {
        --l.readers;
        return Status::Ok;
    }
    return Status::Denied;
}

} // namespace pbsd::userland::libthr
""",
)

# --- libc (+10 to reach 40) ---------------------------------------------------

add(
    "libc/pbsd.userland.libc.gen.getprogname.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.getprogname;

/// getprogname from hbsd/src/lib/libc/gen/getprogname.c
export namespace pbsd::userland::libc {

inline char g_progname_buf[64] = "pbsd";

[[nodiscard]] inline const char* getprogname() noexcept { return g_progname_buf; }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.setprogname.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.setprogname;

import pbsd.userland.libc.gen.getprogname;

/// setprogname from hbsd/src/lib/libc/gen/setprogname.c
export namespace pbsd::userland::libc {

inline void setprogname(const char* name) noexcept {
    const char* src = (name != nullptr && name[0] != '\\0') ? name : "pbsd";
    std::size_t i = 0;
    while (src[i] != '\\0' && i + 1 < sizeof(g_progname_buf)) {
        g_progname_buf[i] = src[i];
        ++i;
    }
    g_progname_buf[i] = '\\0';
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.ldexp.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.libc.gen.ldexp;

/// ldexp from hbsd/src/lib/libc/gen/ldexp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double ldexp(double x, int exp) noexcept { return std::ldexp(x, exp); }

[[nodiscard]] inline float ldexpf(float x, int exp) noexcept { return std::ldexp(x, exp); }

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.freadlink.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.gen.freadlink;

/// freadlink concept from hbsd/src/lib/libc/gen/freadlink.c (logic-only)
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t bounded_readlink_copy(char* buf, std::size_t bufsiz,
                                                         const char* target) noexcept {
    if (buf == nullptr || bufsiz == 0 || target == nullptr) {
        return 0;
    }
    std::size_t i = 0;
    while (target[i] != '\\0' && i + 1 < bufsiz) {
        buf[i] = target[i];
        ++i;
    }
    buf[i] = '\\0';
    return i;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.stdlib.qsort_r.cppm",
    """
module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.qsort_r;

import pbsd.userland.libc.stdlib.sort;

/// qsort_r from hbsd/src/lib/libc/stdlib/qsort_r.c
export namespace pbsd::userland::libc {

using QsortRCompare = int (*)(const void*, const void*, void*) noexcept;

namespace detail {
inline void* g_qsort_r_thunk = nullptr;
inline QsortRCompare g_qsort_r_cmp = nullptr;

inline int qsort_r_bridge(const void* a, const void* b) noexcept {
    return g_qsort_r_cmp(a, b, g_qsort_r_thunk);
}
} // namespace detail

inline void qsort_r(void* base, std::size_t nmemb, std::size_t size, void* thunk,
                    QsortRCompare compar) noexcept {
    detail::g_qsort_r_thunk = thunk;
    detail::g_qsort_r_cmp = compar;
    qsort(base, nmemb, size, detail::qsort_r_bridge);
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.cmp.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.cmp;

/// wcscmp/wcsncmp from hbsd/src/lib/libc/string/{wcscmp,wcsncmp}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wcscmp(const wchar_t* a, const wchar_t* b) noexcept {
    while (*a != L'\\0' && *a == *b) {
        ++a;
        ++b;
    }
    return static_cast<int>(*a) - static_cast<int>(*b);
}

[[nodiscard]] inline int wcsncmp(const wchar_t* a, const wchar_t* b, std::size_t n) noexcept {
    while (n > 0 && *a != L'\\0' && *a == *b) {
        ++a;
        ++b;
        --n;
    }
    if (n == 0) {
        return 0;
    }
    return static_cast<int>(*a) - static_cast<int>(*b);
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.set.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.set;

/// wmemset/wmemchr from hbsd/src/lib/libc/string/{wmemset,wmemchr}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wmemset(wchar_t* dst, wchar_t c, std::size_t n) noexcept {
    wchar_t* p = dst;
    while (n-- > 0) {
        *p++ = c;
    }
    return dst;
}

[[nodiscard]] inline const wchar_t* wmemchr(const wchar_t* s, wchar_t c, std::size_t n) noexcept {
    while (n > 0) {
        if (*s == c) {
            return s;
        }
        ++s;
        --n;
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.string.wchar.span.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.span;

/// wcsspn/wcscspn from hbsd/src/lib/libc/string/{wcsspn,wcscspn}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t wcsspn(const wchar_t* s, const wchar_t* accept) noexcept {
    const wchar_t* p = s;
    while (*p != L'\\0') {
        const wchar_t* a = accept;
        while (*a != L'\\0' && *a != *p) {
            ++a;
        }
        if (*a == L'\\0') {
            break;
        }
        ++p;
    }
    return static_cast<std::size_t>(p - s);
}

[[nodiscard]] inline std::size_t wcscspn(const wchar_t* s, const wchar_t* reject) noexcept {
    const wchar_t* p = s;
    while (*p != L'\\0') {
        for (const wchar_t* r = reject; *r != L'\\0'; ++r) {
            if (*r == *p) {
                return static_cast<std::size_t>(p - s);
            }
        }
        ++p;
    }
    return static_cast<std::size_t>(p - s);
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.locale.wcwidth.cppm",
    """
module;

#include <cwchar>

export module pbsd.userland.libc.locale.wcwidth;

/// wcwidth from hbsd/src/lib/libc/locale/wcwidth.c (ASCII subset)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wcwidth(wchar_t wc) noexcept {
    if (wc == L'\\0') {
        return 0;
    }
    if (wc >= 0x20 && wc < 0x7f) {
        return 1;
    }
    if (wc == L'\\t') {
        return -1;
    }
    return -1;
}

} // namespace pbsd::userland::libc
""",
)

add(
    "libc/pbsd.userland.libc.gen.getpagesize.cppm",
    """
module;

export module pbsd.userland.libc.gen.getpagesize;

/// getpagesize from hbsd/src/lib/libc/gen/getpagesize.c
export namespace pbsd::userland::libc {

inline constexpr int kDefaultPageSize = 4096;

[[nodiscard]] inline int getpagesize() noexcept { return kDefaultPageSize; }

} // namespace pbsd::userland::libc
""",
)

# --- msun (+5 to reach 15) ----------------------------------------------------

add(
    "msun/pbsd.userland.msun.copysign.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.copysign;

/// copysign from hbsd/src/lib/msun/src/s_copysign.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double copysign(double x, double y) noexcept { return std::copysign(x, y); }

[[nodiscard]] inline float copysignf(float x, float y) noexcept { return std::copysign(x, y); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.finite.cppm",
    """
module;

#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.finite;

/// finite from hbsd/src/lib/msun/src/s_finite.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline int finite(double x) noexcept { return std::isfinite(x) ? 1 : 0; }

[[nodiscard]] inline int finitef(float x) noexcept { return std::isfinite(x) ? 1 : 0; }

[[nodiscard]] inline bool finite_bits(std::uint64_t bits) noexcept {
    return ((bits >> 52) & 0x7ff) != 0x7ff;
}

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.lrint.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.lrint;

/// lrint/lrintf from hbsd/src/lib/msun/src/s_lrintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long lrint(double x) noexcept { return std::lrint(x); }

[[nodiscard]] inline long lrintf(float x) noexcept { return std::lrint(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.llround.cppm",
    """
module;

#include <cmath>

export module pbsd.userland.msun.llround;

/// llround/llroundf from hbsd/src/lib/msun/src/s_llroundf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long long llround(double x) noexcept { return std::llround(x); }

[[nodiscard]] inline long long llroundf(float x) noexcept { return std::llround(x); }

} // namespace pbsd::userland::msun
""",
)

add(
    "msun/pbsd.userland.msun.drem.cppm",
    """
module;

#include <cmath>
#include <limits>

export module pbsd.userland.msun.drem;

/// drem/dremf from hbsd/src/lib/msun/src/w_dremf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double drem(double x, double y) noexcept {
    if (y == 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    const double q = std::nearbyint(x / y);
    return x - q * y;
}

[[nodiscard]] inline float dremf(float x, float y) noexcept {
    if (y == 0.0f) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    const float q = std::nearbyint(x / y);
    return x - q * y;
}

} // namespace pbsd::userland::msun
""",
)

# --- libthr (+2 to reach 10) --------------------------------------------------

add(
    "libthr/pbsd.userland.libthr.yield.cppm",
    """
module;

export module pbsd.userland.libthr.yield;

import pbsd.core;

/// pthread_yield from hbsd/src/lib/libthr/thread/thr_yield.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_yield() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
""",
)

add(
    "libthr/pbsd.userland.libthr.affinity.cppm",
    """
module;

export module pbsd.userland.libthr.affinity;

import pbsd.core;
import pbsd.userland.libthr.thread;

/// cpu affinity concepts from hbsd/src/lib/libthr/thread/thr_affinity.c
export namespace pbsd::userland::libthr {

struct CpuSet {
    unsigned mask{0};
};

[[nodiscard]] inline Status affinity_get(ThreadId tid, CpuSet& out) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    out.mask = 1u;
    return Status::Ok;
}

[[nodiscard]] inline Status affinity_set(ThreadId tid, const CpuSet& set) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)set;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
""",
)

# --- rtld (+5) ------------------------------------------------------------------

add(
    "rtld/pbsd.userland.rtld.libmap.cppm",
    """
module;

export module pbsd.userland.rtld.libmap;

/// libmap.conf concepts from hbsd/src/libexec/rtld-elf/libmap.c
export namespace pbsd::userland::rtld {

struct LibmapEntry {
    const char* from{nullptr};
    const char* to{nullptr};
};

namespace detail {
[[nodiscard]] inline bool cstr_eq(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return a == b;
    }
    while (*a != '\\0' && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}
} // namespace detail

[[nodiscard]] inline const char* libmap_lookup(const LibmapEntry* table, std::size_t count,
                                                 const char* name) noexcept {
    if (table == nullptr || name == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (detail::cstr_eq(table[i].from, name)) {
            return table[i].to;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::rtld
""",
)

add(
    "rtld/pbsd.userland.rtld.lock.cppm",
    """
module;

export module pbsd.userland.rtld.lock;

import pbsd.core;

/// rtld lock from hbsd/src/libexec/rtld-elf/rtld_lock.c
export namespace pbsd::userland::rtld {

inline bool g_rtld_locked = false;

[[nodiscard]] inline Status rtld_lock() noexcept {
    if (g_rtld_locked) {
        return Status::Busy;
    }
    g_rtld_locked = true;
    return Status::Ok;
}

[[nodiscard]] inline Status rtld_unlock() noexcept {
    g_rtld_locked = false;
    return Status::Ok;
}

} // namespace pbsd::userland::rtld
""",
)

add(
    "rtld/pbsd.userland.rtld.debug.cppm",
    """
module;

export module pbsd.userland.rtld.debug;

/// rtld debug flags from hbsd/src/libexec/rtld-elf/debug.c
export namespace pbsd::userland::rtld {

enum class DebugFlag : unsigned {
    None = 0,
    Bindings = 0x01,
    Symbols = 0x02,
    Relocations = 0x04,
    All = 0x07,
};

inline unsigned g_debug_flags = 0;

[[nodiscard]] inline bool debug_enabled(DebugFlag f) noexcept {
    return (g_debug_flags & static_cast<unsigned>(f)) != 0;
}

inline void set_debug_flags(unsigned flags) noexcept { g_debug_flags = flags; }

} // namespace pbsd::userland::rtld
""",
)

add(
    "rtld/pbsd.userland.rtld.malloc.cppm",
    """
module;

#include <cstddef>
#include <cstdlib>

export module pbsd.userland.rtld.malloc;

/// rtld_malloc concept from hbsd/src/libexec/rtld-elf/rtld_malloc.c
export namespace pbsd::userland::rtld {

[[nodiscard]] inline void* rtld_malloc(std::size_t size) noexcept {
    return std::malloc(size);
}

inline void rtld_free(void* ptr) noexcept { std::free(ptr); }

} // namespace pbsd::userland::rtld
""",
)

add(
    "rtld/pbsd.userland.rtld.reloc.cppm",
    """
module;

#include <cstdint>

export module pbsd.userland.rtld.reloc;

/// relocation type constants from hbsd/src/libexec/rtld-elf/amd64/reloc.c
export namespace pbsd::userland::rtld {

inline constexpr unsigned kRelocRelative = 8;
inline constexpr unsigned kRelocGlobDat = 6;
inline constexpr unsigned kRelocJumpSlot = 7;
inline constexpr unsigned kRelocTlsTpoFF64 = 18;

struct RelocEntry {
    std::uintptr_t offset{0};
    std::uintptr_t info{0};
    std::intptr_t addend{0};
};

[[nodiscard]] inline unsigned reloc_type(const RelocEntry& r) noexcept {
    return static_cast<unsigned>(r.info & 0xffffffffU);
}

} // namespace pbsd::userland::rtld
""",
)


def main() -> int:
    for rel, body in MODULES.items():
        path = UL / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body, encoding="utf-8")
        print(f"wrote {rel}")
    print(f"total modules: {len(MODULES)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
