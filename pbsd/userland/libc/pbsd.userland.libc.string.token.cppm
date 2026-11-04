module;

export module pbsd.userland.libc.string.token;

/// strsep from hbsd/src/lib/libc/string/strsep.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strsep(char** stringp, const char* delim) noexcept {
    char* s;
    int c;
    int sc;
    char* tok;

    if ((s = *stringp) == nullptr) {
        return nullptr;
    }
    for (tok = s;;) {
        c = *s++;
        const char* spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0) {
                    s = nullptr;
                } else {
                    s[-1] = '\0';
                }
                *stringp = s;
                return tok;
            }
        } while (sc != 0);
    }
}

} // namespace pbsd::userland::libc
