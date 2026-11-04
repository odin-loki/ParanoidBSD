module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kshell;

import pbsd.core;

/// Wave 3 — POSIX shell quoting / tilde helpers (from KShell).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kshell.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kshell {

inline constexpr unsigned kMaxPathLen = 4096;
inline constexpr char kEscape = '\\';

struct QuoteResult {
    char text[kMaxPathLen]{};
    unsigned length{0};
};

namespace detail {

[[nodiscard]] inline bool is_special(char c) noexcept {
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\'':
    case '"':
    case '$':
    case '`':
    case '<':
    case '>':
    case '|':
    case ';':
    case '&':
    case '(':
    case ')':
    case '{':
    case '}':
    case '*':
    case '?':
    case '#':
    case '!':
    case '~':
    case '[':
    case ']':
        return true;
    default:
        return false;
    }
}

} // namespace detail

[[nodiscard]] inline Status quote_arg(const char* arg, QuoteResult& out) noexcept {
    if (arg == nullptr) {
        return Status::Invalid;
    }
    out.length = 0;
    const unsigned len = static_cast<unsigned>(std::strlen(arg));
    if (len == 0) {
        if (out.length + 2 >= kMaxPathLen) {
            return Status::NoMemory;
        }
        out.text[0] = '\'';
        out.text[1] = '\'';
        out.length = 2;
        out.text[2] = '\0';
        return Status::Ok;
    }
    for (unsigned i = 0; i < len; ++i) {
        if (detail::is_special(arg[i])) {
            if (out.length + 1 >= kMaxPathLen) {
                return Status::NoMemory;
            }
            out.text[out.length++] = '\'';
            for (unsigned j = 0; j < len; ++j) {
                if (arg[j] == '\'') {
                    if (out.length + 4 >= kMaxPathLen) {
                        return Status::NoMemory;
                    }
                    out.text[out.length++] = '\'';
                    out.text[out.length++] = '\\';
                    out.text[out.length++] = '\'';
                    out.text[out.length++] = '\'';
                } else {
                    if (out.length + 1 >= kMaxPathLen) {
                        return Status::NoMemory;
                    }
                    out.text[out.length++] = arg[j];
                }
            }
            if (out.length + 1 >= kMaxPathLen) {
                return Status::NoMemory;
            }
            out.text[out.length++] = '\'';
            out.text[out.length] = '\0';
            return Status::Ok;
        }
    }
    if (len >= kMaxPathLen) {
        return Status::NoMemory;
    }
    std::strncpy(out.text, arg, kMaxPathLen - 1);
    out.length = len;
    return Status::Ok;
}

[[nodiscard]] inline bool needs_tilde_expand(const char* path) noexcept {
    return path != nullptr && path[0] == '~';
}

[[nodiscard]] inline bool is_escaped_tilde(const char* path) noexcept {
    return path != nullptr && path[0] == kEscape && path[1] == '~';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kshell.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kshell
