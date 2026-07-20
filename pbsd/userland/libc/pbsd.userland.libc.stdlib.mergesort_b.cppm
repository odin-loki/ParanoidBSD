module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.mergesort_b;

export import pbsd.core;

/// mergesort_b from hbsd/src/lib/libc/stdlib/mergesort_b.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mergesort_b_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
