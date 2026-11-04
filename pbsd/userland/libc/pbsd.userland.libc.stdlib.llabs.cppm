module;

export module pbsd.userland.libc.stdlib.llabs;

/// llabs from hbsd/src/lib/libc/stdlib/llabs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long llabs(long long j) noexcept {
    return j < 0 ? -j : j;
}

} // namespace pbsd::userland::libc
