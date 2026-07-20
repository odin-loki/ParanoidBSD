module;

#include <cstddef>

export module pbsd.userland.libc.string.search.ext;

import pbsd.userland.libc.string;

/// strstr/strnstr from hbsd/src/lib/libc/string/{strstr,strnstr}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strstr(const char* haystack, const char* needle) noexcept {
    if (*needle == '\0') {
        return const_cast<char*>(haystack);
    }
    for (; *haystack != '\0'; ++haystack) {
        if (*haystack != *needle) {
            continue;
        }
        const char* h = haystack;
        const char* n = needle;
        while (*n != '\0' && *h == *n) {
            ++h;
            ++n;
        }
        if (*n == '\0') {
            return const_cast<char*>(haystack);
        }
    }
    return nullptr;
}

[[nodiscard]] inline char* strnstr(const char* s, const char* find, std::size_t slen) noexcept {
    char c;
    char sc;
    std::size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0') {
                    return nullptr;
                }
            } while (sc != c);
            if (len > slen) {
                return nullptr;
            }
        } while (strncmp(s, find, len) != 0);
        --s;
    }
    return const_cast<char*>(s);
}

} // namespace pbsd::userland::libc
