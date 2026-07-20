module;

export module pbsd.userland.libc.string.find;

/// strpbrk from hbsd/src/lib/libc/string/strpbrk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strpbrk(const char* s1, const char* s2) noexcept {
    int c;
    int sc;
    while ((c = *s1++) != 0) {
        for (const char* scanp = s2; (sc = *scanp++) != '\0';) {
            if (sc == c) {
                return const_cast<char*>(s1 - 1);
            }
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
