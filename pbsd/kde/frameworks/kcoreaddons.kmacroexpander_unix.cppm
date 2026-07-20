export module pbsd.kde.frameworks.kcoreaddons.kmacroexpander_unix;

import pbsd.core;
import pbsd.kde.frameworks.kcoreaddons.kshell;

/// Wave 3 — shell macro expander quoting (from kmacroexpander_unix.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/kmacroexpander_unix.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kmacroexpander {

enum class Quoting : unsigned {
    NoQuote = 0,
    SingleQuote,
    DoubleQuote,
    DollarQuote,
    Paren,
    Subst,
    Group,
    Math,
};

struct State {
    Quoting current{Quoting::NoQuote};
    bool dquote{false};
};

[[nodiscard]] inline Status quote_arg(const char* arg, kshell::QuoteResult& out) noexcept {
    return kshell::quote_arg(arg, out);
}

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

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/kmacroexpander_unix.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kmacroexpander
