#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

FILES = {
    "pbsd/userland/usr.bin/pbsd.userland.sed.addr.cppm": """module;
#include <cstddef>

export module pbsd.userland.sed.addr;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_bin::sed::addr {

enum class AddrKind : unsigned char { Line, Regex, Last, Unknown };

struct Address {
    AddrKind kind{AddrKind::Unknown};
    int line{0};
};

[[nodiscard]] inline Result<Address> parse_address(const char* spec) noexcept {
    Address a{};
    if (spec == nullptr || spec[0] == '\\0') {
        return result_err<Address>(Status::Invalid);
    }
    if (spec[0] == '$') {
        a.kind = AddrKind::Last;
        return result_ok(a);
    }
    int n = 0;
    for (const char* p = spec; *p >= '0' && *p <= '9'; ++p) {
        n = n * 10 + (*p - '0');
    }
    if (n > 0) {
        a.kind = AddrKind::Line;
        a.line = n;
        return result_ok(a);
    }
    a.kind = AddrKind::Regex;
    return result_ok(a);
}

}
""",
    "pbsd/userland/usr.bin/pbsd.userland.awk.fields.cppm": """module;
#include <cstddef>

export module pbsd.userland.awk.fields;

export import pbsd.core;

export namespace pbsd::userland::usr_bin::awk::fields {

[[nodiscard]] inline int count_fields(const char* line, char fs) noexcept {
    if (line == nullptr) {
        return 0;
    }
    int count = (*line == '\\0') ? 0 : 1;
    for (const char* p = line; *p; ++p) {
        if (*p == fs) {
            ++count;
        }
    }
    return count;
}

}
""",
    "pbsd/userland/usr.bin/pbsd.userland.xargs.quote.cppm": """module;

export module pbsd.userland.xargs.quote;

export import pbsd.core;

export namespace pbsd::userland::usr_bin::xargs::quote {

[[nodiscard]] inline bool needs_quoting(const char* arg) noexcept {
    if (arg == nullptr) {
        return false;
    }
    for (const char* p = arg; *p; ++p) {
        if (*p == ' ' || *p == '\\t' || *p == '\\'' || *p == '"') {
            return true;
        }
    }
    return false;
}

}
""",
    "pbsd/userland/usr.bin/pbsd.userland.strings.cppm": """module;
#include <cstddef>

export module pbsd.userland.strings;

export import pbsd.core;

export namespace pbsd::userland::usr_bin::strings {

[[nodiscard]] inline bool printable_run(const char* s, int min_len) noexcept {
    if (s == nullptr || min_len <= 0) {
        return false;
    }
    int run = 0;
    for (const char* p = s; *p; ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c >= 0x20 && c < 0x7f) {
            if (++run >= min_len) {
                return true;
            }
        } else {
            run = 0;
        }
    }
    return run >= min_len;
}

}
""",
    "pbsd/userland/usr.bin/pbsd.userland.iconv.cppm": """module;

export module pbsd.userland.iconv;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_bin::iconv {

[[nodiscard]] inline bool charset_is_utf8(const char* cs) noexcept {
    return cs != nullptr
        && (hosted::cstrcmp(cs, "UTF-8") == 0 || hosted::cstrcmp(cs, "utf-8") == 0);
}

}
""",
    "pbsd/userland/usr.sbin/pbsd.userland.periodic.cppm": """module;

export module pbsd.userland.periodic;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_sbin::periodic {

enum class RunMode : unsigned char { Daily, Weekly, Monthly, Security, Unknown };

[[nodiscard]] inline Result<RunMode> mode_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<RunMode>(Status::Invalid);
    }
    if (hosted::cstrcmp(name, "daily") == 0) {
        return result_ok(RunMode::Daily);
    }
    if (hosted::cstrcmp(name, "weekly") == 0) {
        return result_ok(RunMode::Weekly);
    }
    return result_err<RunMode>(Status::Invalid);
}

}
""",
    "pbsd/userland/usr.sbin/pbsd.userland.syslogd.cppm": """module;

export module pbsd.userland.syslogd;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_sbin::syslogd {

[[nodiscard]] inline int priority_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return -1;
    }
    if (hosted::cstrcmp(name, "info") == 0) {
        return 6;
    }
    if (hosted::cstrcmp(name, "debug") == 0) {
        return 7;
    }
    return -1;
}

}
""",
    "pbsd/userland/sbin/pbsd.userland.rcorder.cppm": """module;

export module pbsd.userland.rcorder;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::sbin::rcorder {

[[nodiscard]] inline bool keyword_is_provide(const char* kw) noexcept {
    return kw != nullptr && hosted::cstrcmp(kw, "PROVIDE:") == 0;
}

}
""",
}


def main() -> None:
    for rel, content in FILES.items():
        path = ROOT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {rel}")


if __name__ == "__main__":
    main()
