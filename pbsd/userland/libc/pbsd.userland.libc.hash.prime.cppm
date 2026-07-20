module;

export module pbsd.userland.libc.hash.prime;

/// hash prime sizing from hbsd/src/lib/libc/db/hash/hash.c (prime table)
export namespace pbsd::userland::libc {

inline constexpr unsigned kHashPrimes[] = {
    3, 7, 13, 23, 43, 79, 151, 283, 521, 967, 1801, 3343, 6199, 11497, 21283,
};

[[nodiscard]] inline unsigned hash_next_prime(unsigned min) noexcept {
    for (unsigned p : kHashPrimes) {
        if (p >= min) {
            return p;
        }
    }
    return kHashPrimes[sizeof(kHashPrimes) / sizeof(kHashPrimes[0]) - 1];
}

} // namespace pbsd::userland::libc
