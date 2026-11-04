module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.heapsort_b;

export import pbsd.core;

/// heapsort_b from hbsd/src/lib/libc/stdlib/heapsort_b.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status heapsort_b_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
