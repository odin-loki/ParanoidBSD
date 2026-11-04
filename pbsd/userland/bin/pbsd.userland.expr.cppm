module;
#include <cstddef>

export module pbsd.userland.expr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/expr/expr.y — expression evaluation helpers (logic-only).
export namespace pbsd::userland::bin::expr {

inline constexpr int kErrExit = 2;

enum class ValType : unsigned char {
    Integer,
    NumericString,
    String,
};

struct Options {
    bool nonposix{false};
};

[[nodiscard]] inline bool is_digit_char(char c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] inline bool is_integer(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return false;
    }
    const char* p = s;
    if (*p == '+' || *p == '-') {
        ++p;
    }
    if (*p == '\0') {
        return false;
    }
    for (; *p != '\0'; ++p) {
        if (!is_digit_char(*p)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool is_zero_or_null(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return true;
    }
    if (s[0] == '0' && s[1] == '\0') {
        return true;
    }
    return false;
}

[[nodiscard]] inline Result<long> parse_integer(const char* s) noexcept {
    if (!is_integer(s)) {
        return result_err<long>(Status::Invalid);
    }
    bool negative = false;
    const char* p = s;
    if (*p == '+' || *p == '-') {
        negative = (*p == '-');
        ++p;
    }
    long val = 0;
    for (; *p != '\0'; ++p) {
        val = val * 10 + (*p - '0');
    }
    if (negative) {
        val = -val;
    }
    return result_ok(val);
}

inline void assert_div(long num, long den) noexcept {
    (void)num;
    (void)den;
}

[[nodiscard]] inline int compare_strings(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b);
}

} // namespace pbsd::userland::bin::expr
