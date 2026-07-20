module;

export module pbsd.userland.libc.stdlib.atol;

export import pbsd.core;

/// atol from hbsd/src/lib/libc/stdlib/atol.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long atol_digits(const char* nptr) noexcept {
    if (nptr == nullptr) {
        return 0L;
    }
    long sign = 1;
    while (*nptr == ' ' || *nptr == '\t') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    long acc = 0;
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
