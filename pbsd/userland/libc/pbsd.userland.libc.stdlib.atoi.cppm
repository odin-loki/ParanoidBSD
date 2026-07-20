module;

export module pbsd.userland.libc.stdlib.atoi;

export import pbsd.core;

/// atoi from hbsd/src/lib/libc/stdlib/atoi.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int atoi_digits(const char* nptr) noexcept {
    if (nptr == nullptr) {
        return 0;
    }
    int sign = 1;
    while (*nptr == ' ' || *nptr == '\t') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    int acc = 0;
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
