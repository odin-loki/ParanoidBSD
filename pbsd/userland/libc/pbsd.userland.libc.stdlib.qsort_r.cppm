module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.qsort_r;

import pbsd.userland.libc.stdlib.sort;

/// qsort_r from hbsd/src/lib/libc/stdlib/qsort_r.c
export namespace pbsd::userland::libc {

using QsortRCompare = int (*)(const void*, const void*, void*) noexcept;

namespace detail {
inline void* g_qsort_r_thunk = nullptr;
inline QsortRCompare g_qsort_r_cmp = nullptr;

inline int qsort_r_bridge(const void* a, const void* b) noexcept {
    return g_qsort_r_cmp(a, b, g_qsort_r_thunk);
}
} // namespace detail

inline void qsort_r(void* base, std::size_t nmemb, std::size_t size, void* thunk,
                    QsortRCompare compar) noexcept {
    detail::g_qsort_r_thunk = thunk;
    detail::g_qsort_r_cmp = compar;
    qsort(base, nmemb, size, detail::qsort_r_bridge);
}

} // namespace pbsd::userland::libc
