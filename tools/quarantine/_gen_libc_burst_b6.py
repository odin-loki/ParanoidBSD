#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Wave 2 burst 6 — libc stdio/locale/stdlib + libthr + rtld modules."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def write(rel: str, content: str) -> None:
    p = ROOT / rel
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(content, encoding="utf-8")
    print("wrote", rel)


MODULES: list[tuple[str, str]] = []

# --- stdio (17) ---------------------------------------------------------------
MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.cppm", """export module pbsd.userland.libc.stdio;

export import pbsd.userland.libc.stdio.file;
export import pbsd.userland.libc.stdio.flags;
export import pbsd.userland.libc.stdio.fileno;
export import pbsd.userland.libc.stdio.putc;
export import pbsd.userland.libc.stdio.getc;
export import pbsd.userland.libc.stdio.putchar;
export import pbsd.userland.libc.stdio.getchar;
export import pbsd.userland.libc.stdio.puts;
export import pbsd.userland.libc.stdio.fputs;
export import pbsd.userland.libc.stdio.fputc;
export import pbsd.userland.libc.stdio.fgetc;
export import pbsd.userland.libc.stdio.sprintf;
export import pbsd.userland.libc.stdio.vsnprintf;
export import pbsd.userland.libc.stdio.perror;
export import pbsd.userland.libc.stdio.fflush;
export import pbsd.userland.libc.stdio.fopen;
export import pbsd.userland.libc.stdio.fread;
export import pbsd.userland.libc.stdio.fwrite;

/// stdio helper umbrella (concept FILE; hosted I/O deferred).
export namespace pbsd::userland::libc::stdio {} // namespace
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.file.cppm", """module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.file;

/// Minimal FILE concept from hbsd/src/lib/libc/stdio/local.h (logic-only).
export namespace pbsd::userland::libc::stdio {

inline constexpr unsigned kFlagEof = 0x0010U;
inline constexpr unsigned kFlagErr = 0x0020U;

struct IoFile {
    int fd{-1};
    unsigned flags{0};
    unsigned char* r{nullptr};
    unsigned char* w{nullptr};
    unsigned char* bf{nullptr};
    int bfsize{0};
};

[[nodiscard]] inline bool is_eof(const IoFile& fp) noexcept {
    return (fp.flags & kFlagEof) != 0;
}

[[nodiscard]] inline bool is_err(const IoFile& fp) noexcept {
    return (fp.flags & kFlagErr) != 0;
}

inline void set_eof(IoFile& fp) noexcept { fp.flags |= kFlagEof; }
inline void set_err(IoFile& fp) noexcept { fp.flags |= kFlagErr; }
inline void clear_flags(IoFile& fp) noexcept { fp.flags &= ~(kFlagEof | kFlagErr); }

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.flags.cppm", """module;

export module pbsd.userland.libc.stdio.flags;

import pbsd.userland.libc.stdio.file;

/// feof/ferror/clearerr from hbsd/src/lib/libc/stdio/{feof,ferror,clrerr}.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int feof(IoFile* fp) noexcept {
    return fp != nullptr && is_eof(*fp);
}

[[nodiscard]] inline int ferror(IoFile* fp) noexcept {
    return fp != nullptr && is_err(*fp);
}

inline void clearerr(IoFile* fp) noexcept {
    if (fp != nullptr) {
        clear_flags(*fp);
    }
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fileno.cppm", """module;

export module pbsd.userland.libc.stdio.fileno;

import pbsd.userland.libc.stdio.file;

/// fileno from hbsd/src/lib/libc/stdio/fileno.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fileno(IoFile* fp) noexcept {
    return fp != nullptr ? fp->fd : -1;
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.putc.cppm", """module;

export module pbsd.userland.libc.stdio.putc;

import pbsd.userland.libc.stdio.file;

/// putc concept from hbsd/src/lib/libc/stdio/putc.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int putc(int c, IoFile* fp) noexcept {
    if (fp == nullptr || fp->fd < 0) {
        return -1;
    }
    return c;
}

[[nodiscard]] inline int putc_unlocked(int c, IoFile* fp) noexcept {
    return putc(c, fp);
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.getc.cppm", """module;

export module pbsd.userland.libc.stdio.getc;

import pbsd.userland.libc.stdio.file;

/// getc concept from hbsd/src/lib/libc/stdio/getc.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int getc(IoFile* fp) noexcept {
    if (fp == nullptr || fp->fd < 0 || is_eof(*fp)) {
        return -1;
    }
    return 0;
}

[[nodiscard]] inline int getc_unlocked(IoFile* fp) noexcept {
    return getc(fp);
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.putchar.cppm", """module;

export module pbsd.userland.libc.stdio.putchar;

/// putchar from hbsd/src/lib/libc/stdio/putchar.c (stdout fd=1 concept)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int putchar(int c) noexcept { return c; }

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.getchar.cppm", """module;

export module pbsd.userland.libc.stdio.getchar;

/// getchar from hbsd/src/lib/libc/stdio/getchar.c (stdin fd=0 concept)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int getchar() noexcept { return -1; }

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.puts.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.stdio.puts;

import pbsd.userland.libc.string;

/// puts from hbsd/src/lib/libc/stdio/puts.c (logic-only)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int puts(const char* s) noexcept {
    if (s == nullptr) {
        return -1;
    }
    return static_cast<int>(pbsd::userland::libc::strlen(s)) + 1;
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fputs.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.stdio.fputs;

import pbsd.userland.libc.stdio.file;
import pbsd.userland.libc.string;

/// fputs from hbsd/src/lib/libc/stdio/fputs.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fputs(const char* s, IoFile* fp) noexcept {
    if (s == nullptr || fp == nullptr) {
        return -1;
    }
    return static_cast<int>(pbsd::userland::libc::strlen(s));
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fputc.cppm", """module;

export module pbsd.userland.libc.stdio.fputc;

import pbsd.userland.libc.stdio.putc;

/// fputc from hbsd/src/lib/libc/stdio/fputc.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fputc(int c, IoFile* fp) noexcept { return putc(c, fp); }

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fgetc.cppm", """module;

export module pbsd.userland.libc.stdio.fgetc;

import pbsd.userland.libc.stdio.getc;

/// fgetc from hbsd/src/lib/libc/stdio/fgetc.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fgetc(IoFile* fp) noexcept { return getc(fp); }

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.sprintf.cppm", """module;

#include <cstddef>
#include <cstdio>

export module pbsd.userland.libc.stdio.sprintf;

/// sprintf/snprintf size helpers from hbsd/src/lib/libc/stdio/{sprintf,snprintf}.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int bounded_copy(char* dst, std::size_t size, const char* src) noexcept {
    if (dst == nullptr || size == 0) {
        return 0;
    }
    std::size_t i = 0;
    for (; i + 1 < size && src[i] != '\\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\\0';
    return static_cast<int>(i);
}

[[nodiscard]] inline int snprintf(char* str, std::size_t size, const char* fmt, ...) noexcept {
    (void)fmt;
    return bounded_copy(str, size, "");
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.vsnprintf.cppm", """module;

#include <cstdarg>
#include <cstddef>

export module pbsd.userland.libc.stdio.vsnprintf;

import pbsd.userland.libc.stdio.sprintf;

/// vsnprintf from hbsd/src/lib/libc/stdio/vsnprintf.c (va_list passthrough stub)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int vsnprintf(char* str, std::size_t size, const char* fmt,
                                   va_list ap) noexcept {
    (void)ap;
    return snprintf(str, size, fmt);
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.perror.cppm", """module;

#include <cerrno>

export module pbsd.userland.libc.stdio.perror;

/// perror message helper from hbsd/src/lib/libc/stdio/perror.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline const char* errno_message(int err) noexcept {
    switch (err) {
    case 0: return "Success";
    case EINVAL: return "Invalid argument";
    case ERANGE: return "Result too large";
    case ENOENT: return "No such file or directory";
    default: return "Unknown error";
    }
}

[[nodiscard]] inline const char* current_errno_message() noexcept {
    return errno_message(errno);
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fflush.cppm", """module;

export module pbsd.userland.libc.stdio.fflush;

import pbsd.userland.libc.stdio.file;

/// fflush from hbsd/src/lib/libc/stdio/fflush.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fflush(IoFile* fp) noexcept {
    if (fp == nullptr) {
        return 0;
    }
    if (fp->fd < 0) {
        return -1;
    }
    return 0;
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fopen.cppm", """module;

export module pbsd.userland.libc.stdio.fopen;

import pbsd.userland.libc.stdio.file;

/// fopen mode parse from hbsd/src/lib/libc/stdio/fopen.c (logic-only)
export namespace pbsd::userland::libc::stdio {

enum class OpenMode : unsigned char { Read, Write, Append, ReadWrite, ReadWritePlus };

[[nodiscard]] inline bool parse_fopen_mode(const char* mode, OpenMode& out) noexcept {
    if (mode == nullptr || mode[0] == '\\0') {
        return false;
    }
    switch (mode[0]) {
    case 'r':
        out = (mode[1] == '+') ? OpenMode::ReadWritePlus : OpenMode::Read;
        return true;
    case 'w':
        out = (mode[1] == '+') ? OpenMode::ReadWritePlus : OpenMode::Write;
        return true;
    case 'a':
        out = (mode[1] == '+') ? OpenMode::ReadWritePlus : OpenMode::Append;
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline bool mode_allows_read(OpenMode m) noexcept {
    return m == OpenMode::Read || m == OpenMode::ReadWrite || m == OpenMode::ReadWritePlus;
}

[[nodiscard]] inline bool mode_allows_write(OpenMode m) noexcept {
    return m != OpenMode::Read;
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fread.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.stdio.fread;

import pbsd.userland.libc.stdio.file;

/// fread element count from hbsd/src/lib/libc/stdio/fread.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline std::size_t fread_elements(std::size_t size, std::size_t nmemb) noexcept {
    if (size == 0 || nmemb == 0) {
        return 0;
    }
    return nmemb;
}

[[nodiscard]] inline std::size_t fread_bytes(std::size_t size, std::size_t nmemb) noexcept {
    return size * fread_elements(size, nmemb);
}

[[nodiscard]] inline std::size_t fread(IoFile* fp, void* buf, std::size_t size,
                                         std::size_t nmemb) noexcept {
    (void)fp;
    (void)buf;
    return fread_elements(size, nmemb);
}

} // namespace pbsd::userland::libc::stdio
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdio.fwrite.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.stdio.fwrite;

import pbsd.userland.libc.stdio.file;

/// fwrite element count from hbsd/src/lib/libc/stdio/fwrite.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline std::size_t fwrite(IoFile* fp, const void* buf, std::size_t size,
                                          std::size_t nmemb) noexcept {
    (void)fp;
    (void)buf;
    if (size == 0 || nmemb == 0) {
        return 0;
    }
    return nmemb;
}

} // namespace pbsd::userland::libc::stdio
"""))

# --- ctype ext + locale (9) ---------------------------------------------------
MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.ctype.ext.cppm", """module;

#include <cctype>

export module pbsd.userland.libc.ctype.ext;

/// Extended ctype from hbsd/src/lib/libc/locale/ctype.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int iscntrl(int c) noexcept { return std::iscntrl(c); }
[[nodiscard]] inline int ispunct(int c) noexcept { return std::ispunct(c); }
[[nodiscard]] inline int isprint(int c) noexcept { return std::isprint(c); }
[[nodiscard]] inline int isgraph(int c) noexcept { return std::isgraph(c); }
[[nodiscard]] inline int isblank(int c) noexcept { return std::isblank(c); }

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.cppm", """export module pbsd.userland.libc.locale;

export import pbsd.userland.libc.locale.setlocale;
export import pbsd.userland.libc.locale.localeconv;
export import pbsd.userland.libc.locale.mblen;
export import pbsd.userland.libc.locale.mbstowcs;
export import pbsd.userland.libc.locale.wcstombs;
export import pbsd.userland.libc.locale.nl_langinfo;
export import pbsd.userland.libc.locale.btowc;
export import pbsd.userland.libc.locale.wctob;

/// C-locale stubs from hbsd/src/lib/libc/locale/* (no iconv yet).
export namespace pbsd::userland::libc::locale {} // namespace
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.setlocale.cppm", """module;

export module pbsd.userland.libc.locale.setlocale;

/// setlocale stub from hbsd/src/lib/libc/locale/setlocale.c
export namespace pbsd::userland::libc::locale {

inline constexpr const char* kDefaultLocale = "C";

enum class Category : unsigned char {
    All, Collate, Ctype, Monetary, Numeric, Time, Messages
};

[[nodiscard]] inline const char* setlocale(Category cat, const char* locale) noexcept {
    (void)cat;
    return locale != nullptr ? locale : kDefaultLocale;
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.localeconv.cppm", """module;

export module pbsd.userland.libc.locale.localeconv;

/// localeconv stub from hbsd/src/lib/libc/locale/localeconv.c
export namespace pbsd::userland::libc::locale {

struct LConv {
    const char* decimal_point{".");
    const char* thousands_sep{""};
    const char* grouping{""};
    const char* mon_decimal_point{""};
    const char* mon_thousands_sep{""};
    const char* mon_grouping{""};
    const char* positive_sign{""};
    const char* negative_sign{"-"};
    const char* currency_symbol{""};
    char frac_digits{'\\0'};
    char p_cs_precedes{'\\0'};
    char n_cs_precedes{'\\0'};
    char p_sep_by_space{'\\0'};
    char n_sep_by_space{'\\0'};
    char p_sign_posn{'\\0'};
    char n_sign_posn{'\\0'};
    char int_frac_digits{'\\0'};
    char int_p_cs_precedes{'\\0'};
    char int_n_cs_precedes{'\\0'};
    char int_p_sep_by_space{'\\0'};
    char int_n_sep_by_space{'\\0'};
    char int_p_sign_posn{'\\0'};
    char int_n_sign_posn{'\\0'};
};

[[nodiscard]] inline const LConv& localeconv() noexcept {
    static const LConv c{};
    return c;
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.mblen.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.locale.mblen;

/// mblen stub from hbsd/src/lib/libc/locale/mblen.c (UTF-8 C locale)
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int mblen(const char* s, std::size_t n) noexcept {
    if (s == nullptr) {
        return 0;
    }
    if (n == 0 || s[0] == '\\0') {
        return 0;
    }
    const unsigned char c = static_cast<unsigned char>(s[0]);
    if (c < 0x80) {
        return 1;
    }
    return -1;
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.mbstowcs.cppm", """module;

#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbstowcs;

import pbsd.userland.libc.locale.mblen;

/// mbstowcs stub from hbsd/src/lib/libc/locale/mbstowcs.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbstowcs(wchar_t* pwcs, const char* s, std::size_t n) noexcept {
    if (s == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    while (*s != '\\0') {
        const int len = mblen(s, n);
        if (len <= 0) {
            return static_cast<std::size_t>(-1);
        }
        if (pwcs != nullptr && count < n) {
            pwcs[count] = static_cast<wchar_t>(static_cast<unsigned char>(*s));
        }
        ++count;
        s += len;
    }
    if (pwcs != nullptr && count < n) {
        pwcs[count] = L'\\0';
    }
    return count;
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.wcstombs.cppm", """module;

#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.wcstombs;

/// wcstombs stub from hbsd/src/lib/libc/locale/wcstombs.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t wcstombs(char* s, const wchar_t* pwcs, std::size_t n) noexcept {
    if (pwcs == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    for (; *pwcs != L'\\0'; ++pwcs) {
        if (*pwcs > 0xff) {
            return static_cast<std::size_t>(-1);
        }
        if (s != nullptr) {
            if (count + 1 >= n) {
                return static_cast<std::size_t>(-1);
            }
            s[count] = static_cast<char>(*pwcs);
        }
        ++count;
    }
    if (s != nullptr && count < n) {
        s[count] = '\\0';
    }
    return count;
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.nl_langinfo.cppm", """module;

export module pbsd.userland.libc.locale.nl_langinfo;

/// nl_langinfo keys from hbsd/src/lib/libc/locale/nl_langinfo.c
export namespace pbsd::userland::libc::locale {

enum class NlItem : int {
    DDay1 = 0x20001,
    DDay2 = 0x20002,
    DDay3 = 0x20003,
    DDay4 = 0x20004,
    DDay5 = 0x20005,
    DDay6 = 0x20006,
    DDay7 = 0x20007,
    AbDay1 = 0x20008,
    AbDay2 = 0x20009,
    AbDay3 = 0x2000A,
    AbDay4 = 0x2000B,
    AbDay5 = 0x2000C,
    AbDay6 = 0x2000D,
    AbDay7 = 0x2000E,
    Codeset = 0x2000F,
};

[[nodiscard]] inline const char* nl_langinfo(NlItem item) noexcept {
    switch (item) {
    case NlItem::Codeset: return "UTF-8";
    case NlItem::DDay1: return "Sunday";
    case NlItem::DDay2: return "Monday";
    case NlItem::AbDay1: return "Sun";
    case NlItem::AbDay2: return "Mon";
    default: return "";
    }
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.btowc.cppm", """module;

#include <cwchar>

export module pbsd.userland.libc.locale.btowc;

/// btowc from hbsd/src/lib/libc/locale/btowc.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline wint_t btowc(int c) noexcept {
    if (c == EOF) {
        return WEOF;
    }
    const unsigned char uc = static_cast<unsigned char>(c);
    if (uc >= 0x80) {
        return WEOF;
    }
    return static_cast<wint_t>(uc);
}

} // namespace pbsd::userland::libc::locale
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.locale.wctob.cppm", """module;

#include <cwchar>

export module pbsd.userland.libc.locale.wctob;

/// wctob from hbsd/src/lib/libc/locale/wctob.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int wctob(wint_t wc) noexcept {
    if (wc == WEOF || wc > 0xff) {
        return EOF;
    }
    return static_cast<int>(wc);
}

} // namespace pbsd::userland::libc::locale
"""))

# --- stdlib remaining (11) ----------------------------------------------------
MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.atof.cppm", """module;

#include <cctype>

export module pbsd.userland.libc.stdlib.atof;

import pbsd.userland.libc.stdlib.strtod;

/// atof from hbsd/src/lib/libc/stdlib/atof.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double atof(const char* str) noexcept {
    return strtod(str, nullptr);
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.strtonum.cppm", """module;

#include <climits>
#include <cerrno>
#include <cstdlib>

export module pbsd.userland.libc.stdlib.strtonum;

import pbsd.userland.libc.stdlib.convert.ext;

/// strtonum from hbsd/src/lib/libc/stdlib/strtonum.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtonum(const char* numstr, long long minval, long long maxval,
                                         const char** errstrp) noexcept {
    if (minval > maxval) {
        if (errstrp != nullptr) {
            *errstrp = "invalid";
        }
        errno = EINVAL;
        return 0;
    }
    char* ep = nullptr;
    errno = 0;
    const long long ll = strtoll(numstr, &ep, 10);
    if (errno == EINVAL || numstr == ep || (ep != nullptr && *ep != '\\0')) {
        if (errstrp != nullptr) {
            *errstrp = "invalid";
        }
        errno = EINVAL;
        return 0;
    }
    if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval) {
        if (errstrp != nullptr) {
            *errstrp = "too small";
        }
        errno = ERANGE;
        return 0;
    }
    if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval) {
        if (errstrp != nullptr) {
            *errstrp = "too large";
        }
        errno = ERANGE;
        return 0;
    }
    if (errstrp != nullptr) {
        *errstrp = nullptr;
    }
    return ll;
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.getopt.cppm", """module;

export module pbsd.userland.libc.stdlib.getopt;

/// getopt state from hbsd/src/lib/libc/stdlib/getopt.c
export namespace pbsd::userland::libc {

struct GetoptState {
    int opterr{1};
    int optind{1};
    int optopt{0};
    int optreset{0};
    char* optarg{nullptr};
};

[[nodiscard]] inline int getopt(int argc, char* const argv[], const char* optstring,
                                GetoptState& st) noexcept {
    if (st.optreset != 0 || st.optind >= argc) {
        return -1;
    }
    const char* place = argv[st.optind];
    if (place == nullptr || place[0] != '-' || place[1] == '\\0') {
        return -1;
    }
    if (place[1] == '-' && place[2] == '\\0') {
        ++st.optind;
        return -1;
    }
    const char opt = place[1];
    for (const char* p = optstring; *p != '\\0'; ++p) {
        if (*p == ':') {
            continue;
        }
        if (*p == opt) {
            ++st.optind;
            st.optopt = static_cast<unsigned char>(opt);
            return st.optopt;
        }
    }
    st.optopt = static_cast<unsigned char>(opt);
    return '?';
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.getenv.cppm", """module;

export module pbsd.userland.libc.stdlib.getenv;

import pbsd.userland.libc.string;

/// getenv scan from hbsd/src/lib/libc/stdlib/getenv.c (logic-only)
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* lookup_env(char* const* environ, const char* name) noexcept {
    if (environ == nullptr || name == nullptr) {
        return nullptr;
    }
    const std::size_t nlen = strlen(name);
    for (char* const* ep = environ; *ep != nullptr; ++ep) {
        const char* eq = *ep;
        while (*eq != '\\0' && *eq != '=') {
            ++eq;
        }
        if (*eq != '=') {
            continue;
        }
        if (static_cast<std::size_t>(eq - *ep) == nlen &&
            strncmp(*ep, name, nlen) == 0) {
            return eq + 1;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.heapsort.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.heapsort;

/// heapsort from hbsd/src/lib/libc/stdlib/heapsort.c (sift-down subset)
export namespace pbsd::userland::libc {

template <typename Compare>
inline void heapsort(void* base, std::size_t nmemb, std::size_t size, Compare compar) {
    if (nmemb < 2 || size == 0) {
        return;
    }
    auto* a = static_cast<char*>(base);
    for (std::size_t i = nmemb / 2; i-- > 0;) {
        for (std::size_t child = 2 * i + 1; child < nmemb;) {
            std::size_t best = child;
            if (child + 1 < nmemb &&
                compar(a + (child + 1) * size, a + child * size) > 0) {
                best = child + 1;
            }
            if (compar(a + i * size, a + best * size) >= 0) {
                break;
            }
            for (std::size_t b = 0; b < size; ++b) {
                const char t = a[i * size + b];
                a[i * size + b] = a[best * size + b];
                a[best * size + b] = t;
            }
            i = best;
        }
    }
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.mergesort.cppm", """module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.mergesort;

/// mergesort merge step from hbsd/src/lib/libc/stdlib/merge.c
export namespace pbsd::userland::libc {

template <typename Compare>
inline void merge_ranges(char* base, std::size_t size, std::size_t mid, std::size_t nmemb,
                         Compare compar, char* tmp) {
    std::size_t i = 0;
    std::size_t j = mid;
    std::size_t k = 0;
    while (i < mid && j < nmemb) {
        const char* left = base + i * size;
        const char* right = base + j * size;
        if (compar(left, right) <= 0) {
            for (std::size_t b = 0; b < size; ++b) {
                tmp[k * size + b] = left[b];
            }
            ++i;
        } else {
            for (std::size_t b = 0; b < size; ++b) {
                tmp[k * size + b] = right[b];
            }
            ++j;
        }
        ++k;
    }
    while (i < mid) {
        for (std::size_t b = 0; b < size; ++b) {
            tmp[k * size + b] = base[i * size + b];
        }
        ++i;
        ++k;
    }
    while (j < nmemb) {
        for (std::size_t b = 0; b < size; ++b) {
            tmp[k * size + b] = base[j * size + b];
        }
        ++j;
        ++k;
    }
    for (std::size_t t = 0; t < k * size; ++t) {
        base[t] = tmp[t];
    }
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.imaxdiv.cppm", """module;

#include <cstdint>

export module pbsd.userland.libc.stdlib.imaxdiv;

/// imaxdiv from hbsd/src/lib/libc/stdlib/imaxdiv.c
export namespace pbsd::userland::libc {

struct imaxdiv_t {
    std::intmax_t quot;
    std::intmax_t rem;
};

[[nodiscard]] inline imaxdiv_t imaxdiv(std::intmax_t numer, std::intmax_t denom) noexcept {
    imaxdiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.llabs.cppm", """module;

export module pbsd.userland.libc.stdlib.llabs;

/// llabs from hbsd/src/lib/libc/stdlib/llabs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long llabs(long long j) noexcept {
    return j < 0 ? -j : j;
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.lldiv.cppm", """module;

#include <cstdlib>

export module pbsd.userland.libc.stdlib.lldiv;

/// lldiv from hbsd/src/lib/libc/stdlib/lldiv.c
export namespace pbsd::userland::libc {

struct lldiv_t {
    long long quot;
    long long rem;
};

[[nodiscard]] inline lldiv_t lldiv(long long numer, long long denom) noexcept {
    lldiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.exit.cppm", """module;

export module pbsd.userland.libc.stdlib.exit;

/// exit status helpers from hbsd/src/lib/libc/stdlib/exit.c
export namespace pbsd::userland::libc {

enum class ExitStatus : int {
    Success = 0,
    Failure = 1,
    UsageError = 64,
    DataError = 65,
    NoInput = 66,
    NoUser = 67,
    NoHost = 68,
    Unavailable = 69,
    Software = 70,
    OsError = 71,
    OsFile = 72,
    CantCreate = 73,
    IoError = 74,
    TempFail = 75,
    Protocol = 76,
    NoPerm = 77,
    Config = 78,
};

[[nodiscard]] inline int exit_code(ExitStatus s) noexcept {
    return static_cast<int>(s);
}

} // namespace pbsd::userland::libc
"""))

MODULES.append(("pbsd/userland/libc/pbsd.userland.libc.stdlib.system.cppm", """module;

export module pbsd.userland.libc.stdlib.system;

import pbsd.core;

/// system() concept from hbsd/src/lib/libc/stdlib/system.c (hosted stub)
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status system_command(const char* cmd) noexcept {
    if (cmd == nullptr) {
        return Status::NotImplemented;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libc
"""))

# --- libthr (7) ---------------------------------------------------------------
MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.mutex.cppm", """module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.mutex;

import pbsd.core;

/// pthread_mutex concepts from hbsd/src/lib/libthr/thread/thr_mutex.c
export namespace pbsd::userland::libthr {

using ThreadId = std::uint64_t;
inline constexpr ThreadId kInvalidThread = 0;

enum class MutexType : unsigned char { Normal, Recursive, ErrorCheck };

struct Mutex {
    MutexType type{MutexType::Normal};
    ThreadId owner{kInvalidThread};
    unsigned lock_count{0};
};

[[nodiscard]] inline Status mutex_init(Mutex& m, MutexType t = MutexType::Normal) noexcept {
    m = Mutex{};
    m.type = t;
    return Status::Ok;
}

[[nodiscard]] inline Status mutex_lock(Mutex& m, ThreadId self) noexcept {
    if (self == kInvalidThread) {
        return Status::Invalid;
    }
    if (m.owner == kInvalidThread) {
        m.owner = self;
        m.lock_count = 1;
        return Status::Ok;
    }
    if (m.owner == self && m.type == MutexType::Recursive) {
        ++m.lock_count;
        return Status::Ok;
    }
    if (m.owner == self && m.type == MutexType::ErrorCheck) {
        return Status::Busy;
    }
    return Status::Busy;
}

[[nodiscard]] inline Status mutex_unlock(Mutex& m, ThreadId self) noexcept {
    if (m.owner != self) {
        return Status::Denied;
    }
    if (m.lock_count > 1) {
        --m.lock_count;
        return Status::Ok;
    }
    m.owner = kInvalidThread;
    m.lock_count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status mutex_trylock(Mutex& m, ThreadId self) noexcept {
    if (m.owner != kInvalidThread && m.owner != self) {
        return Status::Busy;
    }
    return mutex_lock(m, self);
}

} // namespace pbsd::userland::libthr
"""))

MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.cond.cppm", """module;

export module pbsd.userland.libthr.cond;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_cond concepts from hbsd/src/lib/libthr/thread/thr_cond.c
export namespace pbsd::userland::libthr {

struct Cond {
    unsigned waiters{0};
    bool signaled{false};
};

[[nodiscard]] inline Status cond_init(Cond& c) noexcept {
    c = Cond{};
    return Status::Ok;
}

[[nodiscard]] inline Status cond_wait(Cond& c, Mutex& m, ThreadId self) noexcept {
    if (m.owner != self) {
        return Status::Denied;
    }
    ++c.waiters;
    m.owner = kInvalidThread;
    m.lock_count = 0;
    if (c.signaled) {
        c.signaled = false;
        --c.waiters;
        return mutex_lock(m, self);
    }
    return Status::Ok;
}

[[nodiscard]] inline Status cond_signal(Cond& c) noexcept {
    if (c.waiters > 0) {
        c.signaled = true;
        --c.waiters;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status cond_broadcast(Cond& c) noexcept {
    c.signaled = c.waiters > 0;
    c.waiters = 0;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
"""))

MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.thread.cppm", """module;

#include <cstdint>

export module pbsd.userland.libthr.thread;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread thread id concepts from hbsd/src/lib/libthr/thread/thr_create.c
export namespace pbsd::userland::libthr {

inline ThreadId g_main_thread{1};

struct ThreadAttr {
    unsigned stack_size{0};
    bool detached{false};
};

[[nodiscard]] inline ThreadId thread_self() noexcept { return g_main_thread; }

[[nodiscard]] inline Status thread_create(ThreadId& out, ThreadAttr attr,
                                            void (*start)(void*), void* arg) noexcept {
    (void)attr;
    (void)start;
    (void)arg;
    out = g_main_thread + 1;
    return Status::NotImplemented;
}

[[nodiscard]] inline Status thread_join(ThreadId tid, void** value_ptr) noexcept {
    (void)tid;
    if (value_ptr != nullptr) {
        *value_ptr = nullptr;
    }
    return Status::NotImplemented;
}

[[nodiscard]] inline Status thread_detach(ThreadId tid) noexcept {
    (void)tid;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
"""))

MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.once.cppm", """module;

export module pbsd.userland.libthr.once;

import pbsd.core;

/// pthread_once from hbsd/src/lib/libthr/thread/thr_once.c
export namespace pbsd::userland::libthr {

struct OnceFlag {
    bool done{false};
};

[[nodiscard]] inline Status once(OnceFlag& flag, void (*init)(void)) noexcept {
    if (!flag.done && init != nullptr) {
        init();
        flag.done = true;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
"""))

MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.key.cppm", """module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.key;

import pbsd.core;

/// pthread key/TSS from hbsd/src/lib/libthr/thread/thr_spec.c
export namespace pbsd::userland::libthr {

inline constexpr std::size_t kMaxKeys = 128;

struct TssKey {
    std::uint32_t id{0};
    bool allocated{false};
    void (*destructor)(void*){nullptr};
};

[[nodiscard]] inline Status key_create(TssKey& key, void (*destructor)(void*)) noexcept {
    key.allocated = true;
    key.destructor = destructor;
    return Status::Ok;
}

[[nodiscard]] inline Status key_delete(TssKey& key) noexcept {
    key = TssKey{};
    return Status::Ok;
}

[[nodiscard]] inline Status key_set(TssKey& key, void* value) noexcept {
    if (!key.allocated) {
        return Status::Invalid;
    }
    (void)value;
    return Status::Ok;
}

[[nodiscard]] inline void* key_get(const TssKey& key) noexcept {
    if (!key.allocated) {
        return nullptr;
    }
    return nullptr;
}

} // namespace pbsd::userland::libthr
"""))

MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.equal.cppm", """module;

export module pbsd.userland.libthr.equal;

import pbsd.userland.libthr.thread;

/// pthread_equal from hbsd/src/lib/libthr/thread/thr_equal.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline bool thread_equal(ThreadId a, ThreadId b) noexcept { return a == b; }

} // namespace pbsd::userland::libthr
"""))

MODULES.append(("pbsd/userland/libthr/pbsd.userland.libthr.symbols.cppm", """module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.symbols;

/// Debugger symbol offsets from hbsd/src/lib/libthr/thread/thr_symbols.c
export namespace pbsd::userland::libthr::symbols {

inline constexpr int kThreadOffTcb = 0;
inline constexpr int kThreadOffTid = 8;
inline constexpr int kThreadOffNext = 16;
inline constexpr int kThreadOffAttrFlags = 24;
inline constexpr int kThreadOffLinkmap = 32;
inline constexpr int kThreadOffTlsIndex = 40;
inline constexpr int kThreadOffReportEvents = 48;
inline constexpr int kThreadOffEventMask = 52;
inline constexpr int kThreadOffEventBuf = 56;
inline constexpr int kThreadSizeKey = 16;
inline constexpr int kThreadOffKeyAllocated = 0;
inline constexpr int kThreadOffKeyDestructor = 8;
inline constexpr int kThreadMaxKeys = 256;
inline constexpr int kThreadOffDtv = 0;
inline constexpr int kThreadOffState = 64;
inline constexpr int kThreadStateRunning = 1;
inline constexpr int kThreadStateZombie = 4;

struct DebugOffsets {
    int off_tcb;
    int off_tid;
    int off_next;
    int off_state;
    int max_keys;
};

[[nodiscard]] inline DebugOffsets default_offsets() noexcept {
    return {kThreadOffTcb, kThreadOffTid, kThreadOffNext, kThreadOffState, kThreadMaxKeys};
}

} // namespace pbsd::userland::libthr::symbols
"""))

# --- rtld (5) -----------------------------------------------------------------
MODULES.append(("pbsd/userland/rtld/pbsd.userland.rtld.symbols.cppm", """module;

export module pbsd.userland.rtld.symbols;

/// Public rtld symbol table from hbsd/src/libexec/rtld-elf/Symbol.map
export namespace pbsd::userland::rtld {

enum class SymbolVersion : unsigned char { Fbsd10, Fbsd13, Fbsd18, Private10 };

struct ExportedSymbol {
    const char* name;
    SymbolVersion version;
};

inline constexpr ExportedSymbol kPublicSymbols[] = {
    {"_rtld_error", SymbolVersion::Fbsd10},
    {"dlclose", SymbolVersion::Fbsd10},
    {"dlerror", SymbolVersion::Fbsd10},
    {"dlopen", SymbolVersion::Fbsd10},
    {"dlsym", SymbolVersion::Fbsd10},
    {"dlfunc", SymbolVersion::Fbsd10},
    {"dlvsym", SymbolVersion::Fbsd10},
    {"dladdr", SymbolVersion::Fbsd10},
    {"dllockinit", SymbolVersion::Fbsd10},
    {"dlinfo", SymbolVersion::Fbsd10},
    {"dl_iterate_phdr", SymbolVersion::Fbsd10},
    {"r_debug_state", SymbolVersion::Fbsd10},
    {"__tls_get_addr", SymbolVersion::Fbsd10},
    {"fdlopen", SymbolVersion::Fbsd13},
    {"rtld_get_var", SymbolVersion::Fbsd18},
    {"rtld_set_var", SymbolVersion::Fbsd18},
};

inline constexpr ExportedSymbol kPrivateSymbols[] = {
    {"_dl_iterate_phdr_locked", SymbolVersion::Private10},
    {"_rtld_thread_init", SymbolVersion::Private10},
    {"_rtld_allocate_tls", SymbolVersion::Private10},
    {"_rtld_free_tls", SymbolVersion::Private10},
    {"_rtld_atfork_pre", SymbolVersion::Private10},
    {"_rtld_atfork_post", SymbolVersion::Private10},
    {"_rtld_addr_phdr", SymbolVersion::Private10},
    {"_rtld_get_pax_flags", SymbolVersion::Private10},
    {"_rtld_get_stack_prot", SymbolVersion::Private10},
    {"_rtld_is_dlopened", SymbolVersion::Private10},
    {"_r_debug_postinit", SymbolVersion::Private10},
    {"_rtld_version__FreeBSD_version", SymbolVersion::Private10},
    {"_rtld_version_laddr_offset", SymbolVersion::Private10},
    {"_rtld_version_dlpi_tls_data", SymbolVersion::Private10},
};

[[nodiscard]] inline bool is_exported(const char* name) noexcept {
    for (const auto& s : kPublicSymbols) {
        if (s.name == name) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool is_private_rtld_symbol(const char* name) noexcept {
    for (const auto& s : kPrivateSymbols) {
        if (s.name == name) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::rtld
"""))

MODULES.append(("pbsd/userland/rtld/pbsd.userland.rtld.dl.cppm", """module;

export module pbsd.userland.rtld.dl;

import pbsd.core;

/// dlopen/dlsym concepts from hbsd/src/libexec/rtld-elf/rtld.c
export namespace pbsd::userland::rtld {

enum class DlMode : unsigned {
    Lazy = 0x001,
    Now = 0x002,
    Local = 0x004,
    Global = 0x010,
};

struct DlHandle {
    unsigned id{0};
    const char* path{nullptr};
};

[[nodiscard]] inline Status dlopen(DlHandle& out, const char* path, DlMode mode) noexcept {
    out.id = 1;
    out.path = path;
    (void)mode;
    return Status::NotImplemented;
}

[[nodiscard]] inline Status dlsym(void*& sym, const DlHandle& handle,
                                    const char* name) noexcept {
    (void)handle;
    (void)name;
    sym = nullptr;
    return Status::NotFound;
}

[[nodiscard]] inline Status dlclose(DlHandle& handle) noexcept {
    handle = DlHandle{};
    return Status::Ok;
}

} // namespace pbsd::userland::rtld
"""))

MODULES.append(("pbsd/userland/rtld/pbsd.userland.rtld.linkmap.cppm", """module;

#include <cstdint>

export module pbsd.userland.rtld.linkmap;

/// Link map entry concept from hbsd/src/libexec/rtld-elf/map_object.c
export namespace pbsd::userland::rtld {

struct LinkMap {
    std::uintptr_t l_addr{0};
    const char* l_name{nullptr};
    std::uintptr_t l_ld{0};
    LinkMap* l_next{nullptr};
    LinkMap* l_prev{nullptr};
};

struct ObjEntry {
    LinkMap linkmap{};
    unsigned ref_count{0};
    bool main_program{false};
};

[[nodiscard]] inline bool is_main_object(const ObjEntry& obj) noexcept {
    return obj.main_program;
}

} // namespace pbsd::userland::rtld
"""))

MODULES.append(("pbsd/userland/rtld/pbsd.userland.rtld.paths.cppm", """module;

export module pbsd.userland.rtld.paths;

/// Default rtld search paths from hbsd/src/libexec/rtld-elf/rtld_paths.h
export namespace pbsd::userland::rtld {

inline constexpr const char* kDefaultLibPath = "/lib:/usr/lib";
inline constexpr const char* kDefaultRtldPath = "/libexec/ld-elf.so.1";
inline constexpr const char* kVarLdLibraryPath = "LD_LIBRARY_PATH";
inline constexpr const char* kVarLdPreload = "LD_PRELOAD";

} // namespace pbsd::userland::rtld
"""))

MODULES.append(("pbsd/userland/rtld/pbsd.userland.rtld.version.cppm", """module;

#include <cstdint>

export module pbsd.userland.rtld.version;

/// rtld version symbols from hbsd/src/libexec/rtld-elf/rtld.c
export namespace pbsd::userland::rtld {

inline constexpr int kFreeBsdVersion = 1405000;
inline constexpr std::uintptr_t kLaddrOffsetVersion = 1;
inline constexpr std::uintptr_t kDlpiTlsDataVersion = 1;

struct RtldVersionInfo {
    int freebsd_version;
    std::uintptr_t laddr_offset;
    std::uintptr_t dlpi_tls_data;
};

[[nodiscard]] inline RtldVersionInfo current_version() noexcept {
    return {kFreeBsdVersion, kLaddrOffsetVersion, kDlpiTlsDataVersion};
}

} // namespace pbsd::userland::rtld
"""))

MODULES.append(("pbsd/userland/rtld/pbsd.userland.rtld.cppm", """export module pbsd.userland.rtld;

export import pbsd.userland.rtld.symbols;
export import pbsd.userland.rtld.dl;
export import pbsd.userland.rtld.linkmap;
export import pbsd.userland.rtld.paths;
export import pbsd.userland.rtld.version;

/// Dynamic linker (rtld-elf) concept stubs — no actual loading yet.
export namespace pbsd::userland::rtld {} // namespace
"""))


def main() -> None:
    for rel, content in MODULES:
        write(rel, content)
    print(f"generated {len(MODULES)} modules")


if __name__ == "__main__":
    main()
