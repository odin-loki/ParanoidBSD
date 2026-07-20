module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.radixsort;

export import pbsd.core;

/// radixsort from hbsd/src/lib/libc/stdlib/radixsort.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status radixsort_base(unsigned char** base, int nmemb) noexcept {
    if (base == nullptr || nmemb < 0) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
