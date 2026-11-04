module;

#include <cstddef>

export module pbsd.userland.libc.string.strtok_r;

import pbsd.userland.libc.string.span;

/// strtok_r from hbsd/src/lib/libc/string/strtok.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strtok_r(char* s, const char* delim,
                                    char** last) noexcept {
    if (s == nullptr && (s = *last) == nullptr) {
        return nullptr;
    }

cont:
    char c = *s++;
    for (const char* spanp = delim; *spanp != '\0'; ++spanp) {
        if (c == *spanp) {
            goto cont;
        }
    }

    if (c == '\0') {
        *last = nullptr;
        return nullptr;
    }
    char* tok = s - 1;

    for (;;) {
        c = *s++;
        const char* spanp = delim;
        do {
            if (*spanp == c) {
                if (c == '\0') {
                    *last = s;
                } else {
                    s[-1] = '\0';
                    *last = s;
                }
                return tok;
            }
        } while (*++spanp != '\0');
    }
}

} // namespace pbsd::userland::libc
