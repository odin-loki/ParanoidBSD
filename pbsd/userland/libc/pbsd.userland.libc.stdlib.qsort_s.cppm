module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.qsort_s;

export import pbsd.core;

/// qsort_s from hbsd/src/lib/libc/stdlib/qsort_s.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status qsort_s_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
