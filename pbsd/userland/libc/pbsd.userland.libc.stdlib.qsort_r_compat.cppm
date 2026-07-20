module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.qsort_r_compat;

export import pbsd.core;

/// qsort_r_compat from hbsd/src/lib/libc/stdlib/qsort_r_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status qsort_r_compat_count(std::size_t nmemb) noexcept { return nmemb == 0 ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
