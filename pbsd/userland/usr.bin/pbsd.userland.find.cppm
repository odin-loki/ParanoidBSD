module;
#include <cstddef>

export module pbsd.userland.find;

export import pbsd.core;

/// Port helpers from hbsd/src/usr.bin/find — primaries parse scaffold.
export namespace pbsd::userland::usr_bin::find {

enum class Primary : unsigned char {
    Name,
    Path,
    Type,
    MaxDepth,
    MinDepth,
    Print,
    Exec,
    Delete,
    Unknown,
};

struct Options {
    bool depth_first{false}; // -d
    bool follow{false};      // -L
    bool xdev{false};        // -x
};

[[nodiscard]] inline Primary parse_primary(const char* tok) noexcept {
    if (tok == nullptr) {
        return Primary::Unknown;
    }
    // compare without hosted dependency
    auto eq = [](const char* a, const char* b) {
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        return *a == *b;
    };
    if (eq(tok, "-name")) {
        return Primary::Name;
    }
    if (eq(tok, "-path")) {
        return Primary::Path;
    }
    if (eq(tok, "-type")) {
        return Primary::Type;
    }
    if (eq(tok, "-maxdepth")) {
        return Primary::MaxDepth;
    }
    if (eq(tok, "-mindepth")) {
        return Primary::MinDepth;
    }
    if (eq(tok, "-print")) {
        return Primary::Print;
    }
    if (eq(tok, "-exec") || eq(tok, "-execdir")) {
        return Primary::Exec;
    }
    if (eq(tok, "-delete")) {
        return Primary::Delete;
    }
    return Primary::Unknown;
}

/// Glob-ish match: only `*` and `?` (no character classes) for -name scaffold.
[[nodiscard]] inline bool glob_match(const char* pat, const char* str) noexcept {
    if (pat == nullptr || str == nullptr) {
        return false;
    }
    while (*pat) {
        if (*pat == '*') {
            ++pat;
            if (*pat == '\0') {
                return true;
            }
            for (; *str; ++str) {
                if (glob_match(pat, str)) {
                    return true;
                }
            }
            return false;
        }
        if (*pat == '?') {
            if (*str == '\0') {
                return false;
            }
            ++pat;
            ++str;
            continue;
        }
        if (*pat != *str) {
            return false;
        }
        ++pat;
        ++str;
    }
    return *str == '\0';
}

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        // stop at first primary-looking token starting with -name etc.
        if (parse_primary(argv[i]) != Primary::Unknown) {
            break;
        }
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'd':
                opt.depth_first = true;
                break;
            case 'L':
                opt.follow = true;
                break;
            case 'P':
                opt.follow = false;
                break;
            case 'x':
                opt.xdev = true;
                break;
            case 'H':
            case 'E':
            case 's':
            case 'f':
                break;
            default:
                // might be start of expression path
                optind_out = i;
                return result_ok(opt);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::find
