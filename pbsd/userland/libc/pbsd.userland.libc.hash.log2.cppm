module;

export module pbsd.userland.libc.hash.log2;

/// hash_log2 from hbsd/src/lib/libc/db/hash/hash_log2.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned hash_log2(unsigned num) noexcept {
    unsigned i = 0;
    unsigned n = num;
    while (n > 1) {
        n >>= 1;
        ++i;
    }
    return i;
}

} // namespace pbsd::userland::libc
