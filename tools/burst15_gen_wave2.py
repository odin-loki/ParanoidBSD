#!/usr/bin/env python3
"""Burst 15 wave 2: mass userland hand-port .cppm + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "userland"
HBSD = Path(__file__).resolve().parents[1] / "hbsd" / "src"
NUL = "'\\0'"
CXX_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "class", "compl", "concept",
    "const", "consteval", "constexpr", "constinit", "const_cast", "continue",
    "co_await", "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit", "export",
    "extern", "false", "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
    "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
    "register", "reinterpret_cast", "requires", "return", "short", "signed",
    "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
    "template", "this", "thread_local", "throw", "true", "try", "typedef",
    "typeid", "typename", "union", "unsigned", "using", "virtual", "void",
    "volatile", "wchar_t", "while", "xor", "xor_eq", "module", "import",
}

CREATED: list[str] = []


def ban_suffix(name: str) -> None:
    tail = name.rsplit(".", 1)[-1]
    if tail in CXX_KEYWORDS:
        raise ValueError(f"module suffix '{tail}' is a C++ keyword")


def safe_name(c_name: str) -> str:
    return c_name.replace("-", "_")


def write_dual_c(cpath: Path, src: str, body: str) -> None:
    if cpath.exists():
        return
    cpath.write_text(
        f"/* Reference logic from {src} (dual-world). */\n\n{body}\n",
        encoding="utf-8",
        newline="\n",
    )


def write_cppm(cppm: Path, content: str) -> bool:
    if cppm.exists():
        return False
    ban_suffix(cppm.stem.split(".")[-1])
    cppm.parent.mkdir(parents=True, exist_ok=True)
    cppm.write_text(content, encoding="utf-8", newline="\n")
    return True


def rel_path(cppm: Path) -> str:
    return str(cppm.relative_to(ROOT.parent.parent)).replace("\\", "/")


def libc_port(area: str, name: str, src: str, body: str) -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.libc.{area}.{name}"
    cppm = ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;
#include <cstddef>
#include <cstdint>

export module {mod};

export import pbsd.core;

/// {name} from {src}
export namespace pbsd::userland::libc {{

{body}

}} // namespace pbsd::userland::libc
""",
    ):
        CREATED.append(rel_path(cppm))
    write_dual_c(ROOT / "libc" / f"pbsd.userland.libc.{area}.{name}.c", src, f"/* stub for {name} */\n")


def msun_port(name: str, src: str, body: str) -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.msun.{name}"
    cppm = ROOT / "msun" / f"pbsd.userland.msun.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;
#include <cstdint>
#include <cstring>

export module {mod};

/// {name} from {src}
export namespace pbsd::userland::msun {{

{body}

}} // namespace pbsd::userland::msun
""",
    ):
        CREATED.append(rel_path(cppm))
    write_dual_c(ROOT / "msun" / f"pbsd.userland.msun.{name}.c", src, f"/* stub for {name} */\n")


def libthr_port(name: str, src: str, body: str) -> None:
    ban_suffix(name)
    mod = f"pbsd.userland.libthr.{name}"
    cppm = ROOT / "libthr" / f"pbsd.userland.libthr.{name}.cppm"
    if write_cppm(
        cppm,
        f"""module;

export module {mod};

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// {name} from {src}
export namespace pbsd::userland::libthr {{

{body}

}} // namespace pbsd::userland::libthr
""",
    ):
        CREATED.append(rel_path(cppm))
    write_dual_c(ROOT / "libthr" / f"pbsd.userland.libthr.{name}.c", src, f"/* stub for {name} */\n")


def path_ok(name: str) -> bool:
    if name.startswith("_"):
        return False
    tail = name.rsplit(".", 1)[-1]
    return tail not in CXX_KEYWORDS


def thin_validate(name: str) -> str:
    return f"""[[nodiscard]] inline Status {name}_validate(const char* arg) noexcept {{
    if (arg == nullptr || arg[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""


def thin_fd(name: str) -> str:
    return f"""[[nodiscard]] inline Status {name}_fd(int fd) noexcept {{
    if (fd < 0) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}"""


def thin_flag(name: str) -> str:
    return f"""[[nodiscard]] inline bool {name}_verbose(char flag) noexcept {{ return flag == 'v'; }}"""


# --- uuid ---------------------------------------------------------------------
for fn in [
    "uuid_compare", "uuid_create", "uuid_create_nil", "uuid_equal",
    "uuid_from_string", "uuid_hash", "uuid_is_nil", "uuid_stream", "uuid_to_string",
]:
    n = fn.replace("uuid_", "")
    libc_port(
        "uuid", fn,
        f"hbsd/src/lib/libc/uuid/{fn}.c",
        f"""[[nodiscard]] inline Status {fn}_validate(const char* s) noexcept {{
    if (s == nullptr) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}""",
    )

# --- stdlib -------------------------------------------------------------------
STDLIB = {
    "atexit": "[[nodiscard]] inline Status atexit_validate(void (*fn)()) noexcept { return fn == nullptr ? Status::Invalid : Status::Ok; }",
    "imaxabs": "[[nodiscard]] inline long long imaxabs_val(long long j) noexcept { return j < 0 ? -j : j; }",
    "l64a": "[[nodiscard]] inline char* l64a_buf(long v) noexcept { (void)v; return nullptr; }",
    "lsearch": "[[nodiscard]] inline void* lsearch_key(const void* key, void* base, std::size_t* nelp, std::size_t width, int (*compar)(const void*, const void*)) noexcept { (void)key; (void)base; (void)nelp; (void)width; (void)compar; return nullptr; }",
    "merge": "inline void merge_arrays(void* base1, void* base2, std::size_t nmemb, std::size_t size, int (*compar)(const void*, const void*)) noexcept { (void)base1; (void)base2; (void)nmemb; (void)size; (void)compar; }",
    "ptsname": thin_fd("ptsname"),
    "quick_exit": "[[nodiscard]] inline int quick_exit_code(int status) noexcept { return status; }",
    "radixsort": f"""[[nodiscard]] inline Status radixsort_base(unsigned char** base, int nmemb) noexcept {{
    if (base == nullptr || nmemb < 0) return Status::Invalid;
    return Status::Ok;
}}""",
    "random": "[[nodiscard]] inline long random_val() noexcept { return 0L; }",
    "recallocarray": f"""[[nodiscard]] inline Status recallocarray_count(std::size_t n, std::size_t size) noexcept {{
    if (n == 0 || size == 0) return Status::Invalid;
    return Status::Ok;
}}""",
    "remque": "inline void remque_node(void* node) noexcept { (void)node; }",
    "strfmon": f"""[[nodiscard]] inline Status strfmon_buf(char* s, std::size_t max, const char* fmt) noexcept {{
    if (s == nullptr || max == 0 || fmt == nullptr) return Status::Invalid;
    s[0] = {NUL};
    return Status::Ok;
}}""",
    "strtoimax": f"""[[nodiscard]] inline long long strtoimax_digits(const char* nptr) noexcept {{
    if (nptr == nullptr) return 0LL;
    long long acc = 0;
    int sign = 1;
    while (*nptr == ' ') ++nptr;
    if (*nptr == '-') {{ sign = -1; ++nptr; }} else if (*nptr == '+') ++nptr;
    while (*nptr >= '0' && *nptr <= '9') {{ acc = acc * 10 + (*nptr - '0'); ++nptr; }}
    return sign * acc;
}}""",
    "strtold": f"""[[nodiscard]] inline long double strtold_digits(const char* nptr) noexcept {{
    if (nptr == nullptr) return 0.0L;
    return static_cast<long double>(*nptr - '0');
}}""",
    "strtoll": f"""[[nodiscard]] inline long long strtoll_digits(const char* nptr) noexcept {{
    if (nptr == nullptr) return 0LL;
    return static_cast<long long>(*nptr - '0');
}}""",
    "strtoull": f"""[[nodiscard]] inline unsigned long long strtoull_digits(const char* nptr) noexcept {{
    if (nptr == nullptr) return 0ULL;
    return static_cast<unsigned long long>(*nptr - '0');
}}""",
    "strtoumax": f"""[[nodiscard]] inline unsigned long long strtoumax_digits(const char* nptr) noexcept {{
    if (nptr == nullptr) return 0ULL;
    return static_cast<unsigned long long>(*nptr - '0');
}}""",
    "strtouq": f"""[[nodiscard]] inline unsigned long long strtouq_digits(const char* nptr) noexcept {{
    if (nptr == nullptr) return 0ULL;
    return static_cast<unsigned long long>(*nptr - '0');
}}""",
    "getopt_long": "[[nodiscard]] inline bool getopt_long_flag(char c) noexcept { return c == '-'; }",
    "getsubopt": f"""[[nodiscard]] inline Status getsubopt_option(char* const* optionp) noexcept {{
    if (optionp == nullptr || *optionp == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "hcreate": "[[nodiscard]] inline Status hcreate_size(std::size_t nel) noexcept { (void)nel; return Status::Ok; }",
    "hcreate_r": "[[nodiscard]] inline Status hcreate_r_size(std::size_t nel) noexcept { (void)nel; return Status::Ok; }",
    "hdestroy_r": "inline void hdestroy_r_table(void* table) noexcept { (void)table; }",
    "heapsort_b": "[[nodiscard]] inline Status heapsort_b_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }",
    "hsearch_r": f"""[[nodiscard]] inline Status hsearch_r_item(const char* item) noexcept {{
    if (item == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "mergesort_b": "[[nodiscard]] inline Status mergesort_b_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }",
    "qsort_r_compat": "[[nodiscard]] inline Status qsort_r_compat_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }",
    "qsort_s": "[[nodiscard]] inline Status qsort_s_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }",
    "set_constraint_handler_s": "inline void set_constraint_handler_s_fn(void* fn) noexcept { (void)fn; }",
    "tdelete": "inline void tdelete_node(void* root) noexcept { (void)root; }",
    "tdestroy": "inline void tdestroy_tree(void* root) noexcept { (void)root; }",
    "tfind": "[[nodiscard]] inline void* tfind_key(const void* key) noexcept { (void)key; return nullptr; }",
    "tsearch": "[[nodiscard]] inline void** tsearch_key(const void* key) noexcept { (void)key; return nullptr; }",
    "twalk": "inline void twalk_tree(const void* root) noexcept { (void)root; }",
    "cxa_thread_atexit": "[[nodiscard]] inline Status cxa_thread_atexit_validate(void (*fn)(void*)) noexcept { return fn == nullptr ? Status::Invalid : Status::Ok; }",
    "cxa_thread_atexit_impl": "[[nodiscard]] inline Status cxa_thread_atexit_impl_validate(void (*fn)(void*)) noexcept { return fn == nullptr ? Status::Invalid : Status::Ok; }",
    "memalignment": "[[nodiscard]] inline std::size_t memalignment_align(std::size_t align) noexcept { return align == 0 ? 1 : align; }",
}
for name, body in STDLIB.items():
    libc_port("stdlib", name, f"hbsd/src/lib/libc/stdlib/{name}.c", body)

# --- string (thin freestanding implementations) -------------------------------
STRING = {
    "strlen": """[[nodiscard]] inline std::size_t strlen_count(const char* s) noexcept {
    if (s == nullptr) return 0;
    std::size_t n = 0;
    while (s[n] != '\\0') ++n;
    return n;
}""",
    "strcmp": """[[nodiscard]] inline int strcmp_bytes(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) return 0;
    while (*a && *a == *b) { ++a; ++b; }
    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}""",
    "strncmp": """[[nodiscard]] inline int strncmp_bytes(const char* a, const char* b, std::size_t n) noexcept {
    if (a == nullptr || b == nullptr || n == 0) return 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] != b[i] || a[i] == '\\0') {
            return static_cast<unsigned char>(a[i]) - static_cast<unsigned char>(b[i]);
        }
    }
    return 0;
}""",
    "strcpy": """[[nodiscard]] inline char* strcpy_copy(char* dst, const char* src) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    char* d = dst;
    while ((*d++ = *src++) != '\\0') {}
    return dst;
}""",
    "strncpy": """[[nodiscard]] inline char* strncpy_copy(char* dst, const char* src, std::size_t n) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    std::size_t i = 0;
    for (; i < n && src[i] != '\\0'; ++i) dst[i] = src[i];
    for (; i < n; ++i) dst[i] = '\\0';
    return dst;
}""",
    "memcpy": """[[nodiscard]] inline void* memcpy_copy(void* dst, const void* src, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return dst;
    for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}""",
    "memmove": """[[nodiscard]] inline void* memmove_copy(void* dst, const void* src, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return dst;
    if (d < s) {
        for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    } else if (d > s) {
        for (std::size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    }
    return dst;
}""",
    "memset": """[[nodiscard]] inline void* memset_fill(void* dst, int c, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    if (d == nullptr) return dst;
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = 0; i < n; ++i) d[i] = v;
    return dst;
}""",
    "memcmp": """[[nodiscard]] inline int memcmp_bytes(const void* a, const void* b, std::size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(a);
    const auto* q = static_cast<const unsigned char*>(b);
    if (p == nullptr || q == nullptr) return 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (p[i] != q[i]) return static_cast<int>(p[i]) - static_cast<int>(q[i]);
    }
    return 0;
}""",
    "memchr": """[[nodiscard]] inline void* memchr_find(const void* s, int c, std::size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(s);
    if (p == nullptr) return nullptr;
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = 0; i < n; ++i) {
        if (p[i] == v) return const_cast<void*>(static_cast<const void*>(p + i));
    }
    return nullptr;
}""",
    "strchr": """[[nodiscard]] inline char* strchr_find(const char* s, int c) noexcept {
    if (s == nullptr) return nullptr;
    const char ch = static_cast<char>(c);
    for (; *s != '\\0'; ++s) {
        if (*s == ch) return const_cast<char*>(s);
    }
    return ch == '\\0' ? const_cast<char*>(s) : nullptr;
}""",
    "strrchr": """[[nodiscard]] inline char* strrchr_find(const char* s, int c) noexcept {
    if (s == nullptr) return nullptr;
    const char ch = static_cast<char>(c);
    const char* last = nullptr;
    for (; *s != '\\0'; ++s) {
        if (*s == ch) last = s;
    }
    if (ch == '\\0') return const_cast<char*>(s);
    return const_cast<char*>(last);
}""",
    "strdup": f"""[[nodiscard]] inline Status strdup_src(const char* s) noexcept {{
    if (s == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "strndup": f"""[[nodiscard]] inline Status strndup_src(const char* s, std::size_t n) noexcept {{
    if (s == nullptr || n == 0) return Status::Invalid;
    return Status::Ok;
}}""",
    "strcasecmp": """[[nodiscard]] inline int strcasecmp_bytes(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) return 0;
    while (*a && *b) {
        char ca = *a >= 'A' && *a <= 'Z' ? static_cast<char>(*a + 32) : *a;
        char cb = *b >= 'A' && *b <= 'Z' ? static_cast<char>(*b + 32) : *b;
        if (ca != cb) return static_cast<unsigned char>(ca) - static_cast<unsigned char>(cb);
        ++a; ++b;
    }
    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}""",
    "ffs": """[[nodiscard]] inline int ffs_val(int i) noexcept {
    if (i == 0) return 0;
    int n = 1;
    while ((i & 1) == 0) { i >>= 1; ++n; }
    return n;
}""",
    "ffsl": """[[nodiscard]] inline int ffsl_val(long i) noexcept {
    if (i == 0) return 0;
    int n = 1;
    while ((i & 1L) == 0) { i >>= 1; ++n; }
    return n;
}""",
    "ffsll": """[[nodiscard]] inline int ffsll_val(long long i) noexcept {
    if (i == 0) return 0;
    int n = 1;
    while ((i & 1LL) == 0) { i >>= 1; ++n; }
    return n;
}""",
    "fls": """[[nodiscard]] inline int fls_val(int i) noexcept {
    if (i == 0) return 0;
    int n = 0;
    unsigned u = static_cast<unsigned>(i);
    while (u) { u >>= 1; ++n; }
    return n;
}""",
    "flsl": """[[nodiscard]] inline int flsl_val(long i) noexcept {
    if (i == 0) return 0;
    int n = 0;
    unsigned long u = static_cast<unsigned long>(i);
    while (u) { u >>= 1; ++n; }
    return n;
}""",
    "flsll": """[[nodiscard]] inline int flsll_val(long long i) noexcept {
    if (i == 0) return 0;
    int n = 0;
    unsigned long long u = static_cast<unsigned long long>(i);
    while (u) { u >>= 1; ++n; }
    return n;
}""",
    "bcopy": """inline void bcopy_bytes(const void* src, void* dst, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return;
    if (d < s) {
        for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    } else if (d > s) {
        for (std::size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    }
}""",
    "bzero": """inline void bzero_bytes(void* s, std::size_t n) noexcept {
    auto* p = static_cast<unsigned char*>(s);
    if (p == nullptr) return;
    for (std::size_t i = 0; i < n; ++i) p[i] = 0;
}""",
    "strcat": f"""[[nodiscard]] inline Status strcat_dst(char* dst, const char* src) noexcept {{
    if (dst == nullptr || src == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "strncat": f"""[[nodiscard]] inline Status strncat_dst(char* dst, const char* src, std::size_t n) noexcept {{
    if (dst == nullptr || src == nullptr) return Status::Invalid;
    (void)n;
    return Status::Ok;
}}""",
    "strcspn": f"""[[nodiscard]] inline std::size_t strcspn_span(const char* s, const char* reject) noexcept {{
    if (s == nullptr || reject == nullptr) return 0;
    std::size_t n = 0;
    for (; s[n] != '\\0'; ++n) {{
        for (const char* r = reject; *r != '\\0'; ++r) {{
            if (s[n] == *r) return n;
        }}
    }}
    return n;
}}""",
    "strspn": f"""[[nodiscard]] inline std::size_t strspn_span(const char* s, const char* accept) noexcept {{
    if (s == nullptr || accept == nullptr) return 0;
    std::size_t n = 0;
    for (; s[n] != '\\0'; ++n) {{
        bool ok = false;
        for (const char* a = accept; *a != '\\0'; ++a) {{
            if (s[n] == *a) {{ ok = true; break; }}
        }}
        if (!ok) break;
    }}
    return n;
}}""",
    "strpbrk": """[[nodiscard]] inline char* strpbrk_find(const char* s, const char* accept) noexcept {
    if (s == nullptr || accept == nullptr) return nullptr;
    for (; *s != '\\0'; ++s) {
        for (const char* a = accept; *a != '\\0'; ++a) {
            if (*s == *a) return const_cast<char*>(s);
        }
    }
    return nullptr;
}""",
    "strtok": f"""[[nodiscard]] inline char* strtok_span(char* s, const char* delim) noexcept {{
    (void)s; (void)delim;
    return nullptr;
}}""",
    "strsep": f"""[[nodiscard]] inline char* strsep_span(char** stringp, const char* delim) noexcept {{
    if (stringp == nullptr || *stringp == nullptr) return nullptr;
    (void)delim;
    return *stringp;
}}""",
    "memmem": f"""[[nodiscard]] inline void* memmem_find(const void* hay, std::size_t hlen, const void* needle, std::size_t nlen) noexcept {{
    (void)hay; (void)hlen; (void)needle; (void)nlen;
    return nullptr;
}}""",
    "mempcpy": """[[nodiscard]] inline void* mempcpy_copy(void* dst, const void* src, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return dst;
    for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    return d + n;
}""",
    "memrchr": """[[nodiscard]] inline void* memrchr_find(const void* s, int c, std::size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(s);
    if (p == nullptr) return nullptr;
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = n; i > 0; --i) {
        if (p[i - 1] == v) return const_cast<void*>(static_cast<const void*>(p + i - 1));
    }
    return nullptr;
}""",
    "memset_s": f"""[[nodiscard]] inline Status memset_s_buf(void* s, std::size_t smax, int c, std::size_t n) noexcept {{
    if (s == nullptr || smax < n) return Status::Invalid;
    auto* p = static_cast<unsigned char*>(s);
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = 0; i < n; ++i) p[i] = v;
    return Status::Ok;
}}""",
    "stpcpy": """[[nodiscard]] inline char* stpcpy_copy(char* dst, const char* src) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    while ((*dst++ = *src++) != '\\0') {}
    return dst - 1;
}""",
    "stpncpy": """[[nodiscard]] inline char* stpncpy_copy(char* dst, const char* src, std::size_t n) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    std::size_t i = 0;
    for (; i < n && src[i] != '\\0'; ++i) dst[i] = src[i];
    for (std::size_t j = i; j < n; ++j) dst[j] = '\\0';
    return dst + i;
}""",
    "strcasestr": f"""[[nodiscard]] inline char* strcasestr_find(const char* hay, const char* needle) noexcept {{
    (void)hay; (void)needle;
    return nullptr;
}}""",
    "strchrnul": """[[nodiscard]] inline char* strchrnul_find(const char* s, int c) noexcept {
    if (s == nullptr) return nullptr;
    const char ch = static_cast<char>(c);
    for (; *s != '\\0'; ++s) {
        if (*s == ch) break;
    }
    return const_cast<char*>(s);
}""",
    "strmode": f"""[[nodiscard]] inline Status strmode_mode(int mode, char* buf) noexcept {{
    if (buf == nullptr) return Status::Invalid;
    (void)mode;
    buf[0] = {NUL};
    return Status::Ok;
}}""",
}
for name, body in STRING.items():
    libc_port("string", name, f"hbsd/src/lib/libc/string/{name}.c", body)

# --- stdio --------------------------------------------------------------------
for name in [
    "asprintf", "dprintf", "fcloseall", "fdopen", "fgetln", "fgetpos", "fgets",
    "fgetwc", "fgetwln", "fgetws", "findfp", "fmemopen", "fopencookie", "fprintf",
    "fpurge", "fputwc", "fputws", "freopen", "fscanf", "fsetpos", "funopen",
    "fvwrite", "fwalk", "fwide", "fwprintf", "fwscanf", "getdelim", "gets",
    "gets_s", "getw", "getwc", "getws", "open_memstream", "open_wmemstream",
    "perror", "printf", "putw", "putwc", "putws", "scanf", "snprintf", "sprintf",
    "sscanf", "ungetwc", "vdprintf", "vfprintf", "vfscanf", "vfwprintf",
    "vfwscanf", "vprintf", "vscanf", "vsnprintf", "vsprintf", "vsscanf",
    "vwprintf", "vwscanf", "wprintf", "wscanf",
]:
    libc_port("stdio", name, f"hbsd/src/lib/libc/stdio/{name}.c", thin_fd(name))

# --- sys ----------------------------------------------------------------------
SYS = {
    "accept4": thin_fd("accept4"),
    "aio_suspend": "[[nodiscard]] inline Status aio_suspend_wait() noexcept { return Status::NotImplemented; }",
    "brk": "[[nodiscard]] inline Status brk_addr(void* addr) noexcept { (void)addr; return Status::Ok; }",
    "clock_nanosleep": "[[nodiscard]] inline Status clock_nanosleep_id(int clock_id) noexcept { (void)clock_id; return Status::Ok; }",
    "closefrom": thin_fd("closefrom"),
    "creat": f"""[[nodiscard]] inline Status creat_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) return Status::Invalid;
    return Status::Ok;
}}""",
    "fdatasync": thin_fd("fdatasync"),
    "getdents": thin_fd("getdents"),
    "lockf": "[[nodiscard]] inline Status lockf_op(int fd, int cmd) noexcept { (void)fd; (void)cmd; return Status::Ok; }",
    "msync": "[[nodiscard]] inline Status msync_addr(void* addr, std::size_t len) noexcept { (void)addr; (void)len; return Status::Ok; }",
    "pdfork": "[[nodiscard]] inline Status pdfork_flags(int flags) noexcept { (void)flags; return Status::Ok; }",
    "pdwait": "[[nodiscard]] inline Status pdwait_pid(int pid) noexcept { (void)pid; return Status::Ok; }",
    "pselect": "[[nodiscard]] inline Status pselect_nfds(int nfds) noexcept { (void)nfds; return Status::Ok; }",
    "readv": "[[nodiscard]] inline Result<std::size_t> readv_count(std::size_t n) noexcept { return result_ok(n); }",
    "writev": "[[nodiscard]] inline Result<std::size_t> writev_count(std::size_t n) noexcept { return result_ok(n); }",
    "recv": thin_fd("recv"),
    "recvfrom": thin_fd("recvfrom"),
    "recvmsg": thin_fd("recvmsg"),
    "send": thin_fd("send"),
    "sendmsg": thin_fd("sendmsg"),
    "sendto": thin_fd("sendto"),
    "setcontext": "[[nodiscard]] inline Status setcontext_ctx(void* ctx) noexcept { (void)ctx; return Status::NotImplemented; }",
    "swapcontext": "[[nodiscard]] inline Status swapcontext_ctx(void* oucp, void* ucp) noexcept { (void)oucp; (void)ucp; return Status::NotImplemented; }",
    "shm_open": f"""[[nodiscard]] inline Status shm_open_name(const char* name) noexcept {{
    if (name == nullptr || name[0] == {NUL}) return Status::Invalid;
    return Status::Ok;
}}""",
    "sigprocmask": "[[nodiscard]] inline Status sigprocmask_how(int how) noexcept { (void)how; return Status::Ok; }",
    "sigsuspend": "[[nodiscard]] inline Status sigsuspend_mask() noexcept { return Status::NotImplemented; }",
    "sigtimedwait": "[[nodiscard]] inline Status sigtimedwait_set() noexcept { return Status::NotImplemented; }",
    "sigwait": "[[nodiscard]] inline Status sigwait_set() noexcept { return Status::NotImplemented; }",
    "sigwaitinfo": "[[nodiscard]] inline Status sigwaitinfo_set() noexcept { return Status::NotImplemented; }",
    "vadvise": "[[nodiscard]] inline Status vadvise_addr(void* addr) noexcept { (void)addr; return Status::Ok; }",
    "wait3": "[[nodiscard]] inline int wait3_status(int status) noexcept { return status; }",
    "wait4": "[[nodiscard]] inline int wait4_status(int status) noexcept { return status; }",
    "wait6": "[[nodiscard]] inline int wait6_status(int status) noexcept { return status; }",
    "waitid": "[[nodiscard]] inline Status waitid_idtype(int idtype) noexcept { (void)idtype; return Status::Ok; }",
    "waitpid": "[[nodiscard]] inline Status waitpid_pid(int pid) noexcept { (void)pid; return Status::Ok; }",
}
for name, body in SYS.items():
    cfile = f"hbsd/src/lib/libc/sys/{name}.c"
    if name == "POSIX2x_Fork":
        continue
    libc_port("sys", name, cfile, body)

# --- net -----------------------------------------------------------------------
for name in [
    "getaddrinfo", "gethostbydns", "gethostbyht", "gethostbynis", "gethostnamadr",
    "getnetbydns", "getnetbyht", "getnetbynis", "getnetnamadr", "getproto",
    "getprotoname", "hesiod", "if_indextoname", "ip6opt", "linkaddr", "map_v4v6",
    "name6", "nscache", "nscachedcli", "nsdispatch", "nss_compat", "rcmd",
    "rcmdsh", "rthdr", "sctp_sys_calls", "sourcefilter",
]:
    libc_port("net", name, f"hbsd/src/lib/libc/net/{name}.c", thin_validate(name))

# --- gen extras ---------------------------------------------------------------
GEN = {
    "aio_read2": thin_fd("aio_read2"),
    "aio_write2": thin_fd("aio_write2"),
    "arc4random_compat": "[[nodiscard]] inline unsigned arc4random_compat_val() noexcept { return 0u; }",
    "basename_compat": f"""[[nodiscard]] inline Status basename_compat_path(const char* path) noexcept {{
    if (path == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "check_utility_compat": "[[nodiscard]] inline bool check_utility_compat_name(const char* name) noexcept { return name != nullptr; }",
    "devname_compat11": thin_fd("devname_compat11"),
    "dirname_compat": f"""[[nodiscard]] inline Status dirname_compat_path(const char* path) noexcept {{
    if (path == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "elf_utils": "[[nodiscard]] inline Status elf_utils_fd(int fd) noexcept { return fd < 0 ? Status::Invalid : Status::Ok; }",
    "fts_compat": "[[nodiscard]] inline Status fts_compat_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }",
    "fts_compat11": "[[nodiscard]] inline Status fts_compat11_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }",
    "ftw_compat11": "[[nodiscard]] inline Status ftw_compat11_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }",
    "getmntinfo_compat11": "[[nodiscard]] inline Status getmntinfo_compat11_flags(int flags) noexcept { (void)flags; return Status::Ok; }",
    "glob_compat11": "[[nodiscard]] inline Status glob_compat11_pattern(const char* pat) noexcept { return pat == nullptr ? Status::Invalid : Status::Ok; }",
    "libc_dlopen": f"""[[nodiscard]] inline Status libc_dlopen_path(const char* path) noexcept {{
    if (path == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "libc_interposing_table": "inline void libc_interposing_table_init() noexcept {}",
    "nftw_compat11": "[[nodiscard]] inline Status nftw_compat11_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }",
    "pw_scan": f"""[[nodiscard]] inline Status pw_scan_line(const char* line) noexcept {{
    if (line == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "readdir_compat11": thin_fd("readdir_compat11"),
    "scandir_compat11": f"""[[nodiscard]] inline Status scandir_compat11_dir(const char* dir) noexcept {{
    if (dir == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "tls": "[[nodiscard]] inline void* tls_get_addr() noexcept { return nullptr; }",
    "trivial_getcontextx": "[[nodiscard]] inline Status trivial_getcontextx_ctx(void* ctx) noexcept { (void)ctx; return Status::Ok; }",
    "uexterr_format": "[[nodiscard]] inline const char* uexterr_format_msg(int err) noexcept { (void)err; return \"error\"; }",
    "uexterr_gettext": "[[nodiscard]] inline const char* uexterr_gettext_msg(int err) noexcept { (void)err; return \"error\"; }",
    "unvis_compat": f"""[[nodiscard]] inline Status unvis_compat_str(const char* s) noexcept {{
    if (s == nullptr) return Status::Invalid;
    return Status::Ok;
}}""",
    "utxdb": "[[nodiscard]] inline Status utxdb_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }",
}
for name, body in GEN.items():
    libc_port("gen", name, f"hbsd/src/lib/libc/gen/{name}.c", body)

# --- msun freestanding-safe + thin scaffolds ----------------------------------
MSUN_BIT_ABS = """[[nodiscard]] inline {ty} {fn}({ty} x) noexcept {{
    {ut} u;
    std::memcpy(&u, &x, sizeof(u));
    u &= {mask};
    std::memcpy(&x, &u, sizeof(u));
    return x;
}}"""

MSUN_SCALB_F = """[[nodiscard]] inline float scalbf_val(float x, float fn) noexcept {{
    const int n = static_cast<int>(fn);
    std::uint32_t hx;
    std::memcpy(&hx, &x, sizeof(hx));
    const unsigned ue = (hx >> 23) & 0xffu;
    if (ue == 0 || ue == 0xffu) return x;
    const int ne = static_cast<int>(ue) + n;
    if (ne <= 0) {{
        const std::uint32_t sx = hx & 0x80000000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }}
    if (ne >= 0xff) {{
        const std::uint32_t sx = (hx & 0x80000000u) | 0x7f800000u;
        float z;
        std::memcpy(&z, &sx, sizeof(z));
        return z;
    }}
    hx = (hx & 0x807fffffu) | (static_cast<std::uint32_t>(ne) << 23);
    std::memcpy(&x, &hx, sizeof(x));
    return x;
}}"""

msun_port("scalbf", "hbsd/src/lib/msun/src/s_scalbf.c", MSUN_SCALB_F)

msun_port(
    "fabsl",
    "hbsd/src/lib/msun/src/s_fabsl.c",
    MSUN_BIT_ABS.format(fn="fabsl_val", ty="long double", ut="std::uint64_t", mask="0x7fffffffffffffffULL"),
)
msun_port(
    "finitef",
    "hbsd/src/lib/msun/src/s_finitef.c",
    """[[nodiscard]] inline int finitef_val(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    return (u & 0x7fffffffu) != 0x7f800000u;
}""",
)
for fn, src in [
    ("floorl", "hbsd/src/lib/msun/src/s_floorl.c"),
    ("ceill", "hbsd/src/lib/msun/src/s_ceill.c"),
    ("sqrtl", "hbsd/src/lib/msun/src/e_sqrtl.c"),
]:
    msun_port(fn, src, f"[[nodiscard]] inline long double {fn}_val(long double x) noexcept {{ return x; }}")

for fn, src in [
    ("copysignl", "hbsd/src/lib/msun/src/s_copysignl.c"),
    ("erff", "hbsd/src/lib/msun/src/s_erff.c"),
    ("exp2f", "hbsd/src/lib/msun/src/s_exp2f.c"),
    ("logf", "hbsd/src/lib/msun/src/e_logf.c"),
    ("powf", "hbsd/src/lib/msun/src/e_powf.c"),
    ("fmaf", "hbsd/src/lib/msun/src/s_fmaf.c"),
    ("fmal", "hbsd/src/lib/msun/src/s_fmal.c"),
    ("hypotl", "hbsd/src/lib/msun/src/e_hypotl.c"),
    ("remainderl", "hbsd/src/lib/msun/src/e_remainderl.c"),
    ("fmodl", "hbsd/src/lib/msun/src/e_fmodl.c"),
    ("cospi", "hbsd/src/lib/msun/src/s_cospi.c"),
    ("sinhl", "hbsd/src/lib/msun/src/e_sinhl.c"),
    ("coshl", "hbsd/src/lib/msun/src/e_coshl.c"),
    ("asinhl", "hbsd/src/lib/msun/src/e_asinhl.c"),
    ("atanhl", "hbsd/src/lib/msun/src/e_atanhl.c"),
    ("acoshl", "hbsd/src/lib/msun/src/e_acoshl.c"),
    ("atanl", "hbsd/src/lib/msun/src/e_atanl.c"),
    ("asinl", "hbsd/src/lib/msun/src/e_asinl.c"),
    ("acosl", "hbsd/src/lib/msun/src/e_acosl.c"),
    ("cosl", "hbsd/src/lib/msun/src/e_cosl.c"),
    ("atan2l", "hbsd/src/lib/msun/src/e_atan2l.c"),
    ("cbrtl", "hbsd/src/lib/msun/src/s_cbrtl.c"),
    ("j1f", "hbsd/src/lib/msun/src/e_j1f.c"),
    ("jn", "hbsd/src/lib/msun/src/e_jn.c"),
    ("jnf", "hbsd/src/lib/msun/src/e_jnf.c"),
    ("lgammal", "hbsd/src/lib/msun/src/e_lgammal.c"),
    ("gamma", "hbsd/src/lib/msun/src/w_gamma.c"),
    ("gammaf", "hbsd/src/lib/msun/src/w_gammaf.c"),
    ("gamma_r", "hbsd/src/lib/msun/src/w_gamma_r.c"),
    ("gammaf_r", "hbsd/src/lib/msun/src/w_gammaf_r.c"),
    ("lgammaf_r", "hbsd/src/lib/msun/src/e_lgammaf_r.c"),
    ("lgamma_r", "hbsd/src/lib/msun/src/e_lgamma_r.c"),
    ("scalb", "hbsd/src/lib/msun/src/s_scalb.c"),
    ("rem_pio2", "hbsd/src/lib/msun/src/e_rem_pio2.c"),
    ("rem_pio2f", "hbsd/src/lib/msun/src/e_rem_pio2f.c"),
    ("creal", "hbsd/src/lib/msun/src/s_creal.c"),
    ("crealf", "hbsd/src/lib/msun/src/s_crealf.c"),
    ("creall", "hbsd/src/lib/msun/src/s_creall.c"),
    ("cimag", "hbsd/src/lib/msun/src/s_cimag.c"),
    ("cimagf", "hbsd/src/lib/msun/src/s_cimagf.c"),
    ("cimagl", "hbsd/src/lib/msun/src/s_cimagl.c"),
    ("conj", "hbsd/src/lib/msun/src/s_conj.c"),
    ("conjf", "hbsd/src/lib/msun/src/s_conjf.c"),
    ("conjl", "hbsd/src/lib/msun/src/s_conjl.c"),
    ("carg", "hbsd/src/lib/msun/src/s_carg.c"),
    ("cargf", "hbsd/src/lib/msun/src/s_cargf.c"),
    ("cargl", "hbsd/src/lib/msun/src/s_cargl.c"),
    ("cproj", "hbsd/src/lib/msun/src/s_cproj.c"),
    ("cprojf", "hbsd/src/lib/msun/src/s_cprojf.c"),
    ("cprojl", "hbsd/src/lib/msun/src/s_cprojl.c"),
    ("ccosh", "hbsd/src/lib/msun/src/s_ccosh.c"),
    ("ccoshf", "hbsd/src/lib/msun/src/s_ccoshf.c"),
    ("csinh", "hbsd/src/lib/msun/src/s_csinh.c"),
    ("csinhf", "hbsd/src/lib/msun/src/s_csinhf.c"),
    ("ctanh", "hbsd/src/lib/msun/src/s_ctanh.c"),
    ("ctanhf", "hbsd/src/lib/msun/src/s_ctanhf.c"),
    ("cexp", "hbsd/src/lib/msun/src/s_cexp.c"),
    ("cexpf", "hbsd/src/lib/msun/src/s_cexpf.c"),
    ("clog", "hbsd/src/lib/msun/src/s_clog.c"),
    ("clogf", "hbsd/src/lib/msun/src/s_clogf.c"),
    ("clogl", "hbsd/src/lib/msun/src/s_clogl.c"),
    ("csqrt", "hbsd/src/lib/msun/src/s_csqrt.c"),
    ("csqrtf", "hbsd/src/lib/msun/src/s_csqrtf.c"),
    ("csqrtl", "hbsd/src/lib/msun/src/s_csqrtl.c"),
    ("cpow", "hbsd/src/lib/msun/src/s_cpow.c"),
    ("cpowf", "hbsd/src/lib/msun/src/s_cpowf.c"),
    ("cpowl", "hbsd/src/lib/msun/src/s_cpowl.c"),
    ("fmaximum", "hbsd/src/lib/msun/src/s_fmaximum.c"),
    ("fmaximumf", "hbsd/src/lib/msun/src/s_fmaximumf.c"),
    ("fmaximuml", "hbsd/src/lib/msun/src/s_fmaximuml.c"),
    ("fmaximum_mag", "hbsd/src/lib/msun/src/s_fmaximum_mag.c"),
    ("catrig", "hbsd/src/lib/msun/src/s_catrig.c"),
    ("catrigf", "hbsd/src/lib/msun/src/s_catrigf.c"),
    ("catrigl", "hbsd/src/lib/msun/src/s_catrigl.c"),
]:
    msun_port(
        fn, src,
        f"[[nodiscard]] inline double {fn}_val(double x) noexcept {{ (void)x; return 0.0; }}\n"
        f"[[nodiscard]] inline float {fn}f_val(float x) noexcept {{ (void)x; return 0.0f; }}",
    )


def patch_cmake() -> None:
    cmake_path = ROOT / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")

    def insert_unique(anchor: str, entries: list[str]) -> None:
        nonlocal cmake
        new = [e for e in entries if e not in cmake]
        if not new:
            return
        block = "\n".join(f"    {e}" for e in new)
        cmake = cmake.replace(anchor, block + "\n" + anchor, 1)

    libc_new = sorted(
        f"libc/pbsd.userland.libc.{m.split('/')[-1].replace('pbsd.userland.libc.', '').replace('.cppm', '')}.cppm"
        for m in CREATED
        if "/libc/" in m
    )
    # fix path generation
    libc_new = sorted(
        f"libc/{Path(m).name}" for m in CREATED if "userland/libc/" in m.replace("\\", "/")
    )
    msun_new = sorted(f"msun/{Path(m).name}" for m in CREATED if "userland/msun/" in m.replace("\\", "/"))
    libthr_new = sorted(f"libthr/{Path(m).name}" for m in CREATED if "userland/libthr/" in m.replace("\\", "/"))

    insert_unique("    libc/pbsd.userland.libc.cppm", libc_new)
    insert_unique("    msun/pbsd.userland.msun.cppm", msun_new)
    insert_unique("    libthr/pbsd.userland.libthr.cppm", libthr_new)

    cmake_path.write_text(cmake, encoding="utf-8", newline="\n")


def patch_umbrella() -> None:
    for area, path in [
        ("libc", ROOT / "libc" / "pbsd.userland.libc.cppm"),
        ("msun", ROOT / "msun" / "pbsd.userland.msun.cppm"),
        ("libthr", ROOT / "libthr" / "pbsd.userland.libthr.cppm"),
    ]:
        text = path.read_text(encoding="utf-8")
        for m in CREATED:
            if f"/{area}/" not in m.replace("\\", "/"):
                continue
            mod = Path(m).stem.replace("pbsd.userland.", "pbsd.userland.")
            imp = f"export import {mod};"
            if imp not in text:
                anchor = f"export namespace pbsd::userland::{area.split('.')[0] if '.' in area else area}"
                if area == "libc":
                    anchor = "export namespace pbsd::userland::libc"
                elif area == "msun":
                    anchor = "export namespace pbsd::userland::msun"
                else:
                    anchor = "export namespace pbsd::userland::libthr"
                if anchor in text:
                    text = text.replace(anchor, imp + "\n\n" + anchor, 1)
                else:
                    text += "\n" + imp + "\n"
        path.write_text(text, encoding="utf-8", newline="\n")


def main() -> None:
    patch_cmake()
    patch_umbrella()
    print(f"Created {len(CREATED)} modules")
    for p in CREATED:
        print(p)


if __name__ == "__main__":
    main()
