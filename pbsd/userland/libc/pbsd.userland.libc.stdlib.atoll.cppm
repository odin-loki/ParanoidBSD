module;

export module pbsd.userland.libc.stdlib.atoll;

export import pbsd.core;

/// atoll from hbsd/src/lib/libc/stdlib/atoll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long atoll_digits(const char* nptr) noexcept {
    if (nptr == nullptr) {
        return 0LL;
    }
    long long sign = 1;
    while (*nptr == ' ' || *nptr == '\t') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    long long acc = 0;
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
