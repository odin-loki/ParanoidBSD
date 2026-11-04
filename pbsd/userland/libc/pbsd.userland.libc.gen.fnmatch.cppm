module;
#include <cstddef>

export module pbsd.userland.libc.gen.fnmatch;

export import pbsd.core;

/// fnmatch from hbsd/src/lib/libc/gen/fnmatch.c (ASCII subset).
export namespace pbsd::userland::libc {

inline constexpr int kFnmNoEscape = 0x01;
inline constexpr int kFnmPathname = 0x02;
inline constexpr int kFnmPeriod = 0x04;
inline constexpr int kFnmLeadingDir = 0x08;
inline constexpr int kFnmCasefold = 0x10;

[[nodiscard]] inline char fold_char(char c, int flags) noexcept {
    if ((flags & kFnmCasefold) == 0) {
        return c;
    }
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c + ('a' - 'A'));
    }
    return c;
}

[[nodiscard]] inline bool match_star(const char* pattern, const char* string, int flags) noexcept {
    if (pattern == nullptr || string == nullptr) {
        return false;
    }
    if ((flags & kFnmPathname) != 0) {
        const char* slash = string;
        while (*slash != '\0' && *slash != '/') {
            ++slash;
        }
        if (*pattern == '\0') {
            return *slash == '\0';
        }
    }
    if (*pattern == '\0') {
        return true;
    }
    for (const char* s = string; *s != '\0'; ++s) {
        if (match_star(pattern, s, flags)) {
            return true;
        }
        if ((flags & kFnmPathname) != 0 && *s == '/') {
            break;
        }
    }
    return *pattern == '\0' && *string == '\0';
}

[[nodiscard]] inline Result<int> fnmatch(const char* pattern, const char* string,
                                         int flags) noexcept {
    if (pattern == nullptr || string == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    while (*pattern != '\0') {
        if (*pattern == '*') {
            ++pattern;
            return match_star(pattern, string, flags) ? result_ok(0) : result_ok(1);
        }
        if (*string == '\0') {
            return result_ok(1);
        }
        if (*pattern == '?' && ((flags & kFnmPathname) == 0 || *string != '/')) {
            ++pattern;
            ++string;
            continue;
        }
        if (fold_char(*pattern, flags) != fold_char(*string, flags)) {
            return result_ok(1);
        }
        ++pattern;
        ++string;
    }
    return result_ok(*string == '\0' ? 0 : 1);
}

} // namespace pbsd::userland::libc
