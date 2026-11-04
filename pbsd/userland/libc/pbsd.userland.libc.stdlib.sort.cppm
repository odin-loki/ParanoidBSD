module;

#include <cstddef>
#include <cstdlib>

export module pbsd.userland.libc.stdlib.sort;

/// qsort/bsearch from hbsd/src/lib/libc/stdlib/{qsort,bsearch}.c
export namespace pbsd::userland::libc {

using QsortCmp = int (*)(const void*, const void*);
using BsearchCmp = int (*)(const void*, const void*);

namespace detail {

inline void swapfunc(char* a, char* b, std::size_t es) noexcept {
    do {
        const char t = *a;
        *a++ = *b;
        *b++ = t;
    } while (--es > 0);
}

inline char* med3(char* a, char* b, char* c, QsortCmp cmp) noexcept {
    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}

inline void local_qsort(void* a, std::size_t n, std::size_t es, QsortCmp cmp) noexcept {
    char* pa;
    char* pb;
    char* pc;
    char* pd;
    char* pl;
    char* pm;
    char* pn;
    std::size_t d1;
    std::size_t d2;
    int cmp_result;

    if (n < 2) {
        return;
    }
loop:
    if (n < 7) {
        for (pm = static_cast<char*>(a) + es; pm < static_cast<char*>(a) + n * es;
             pm += es) {
            for (pl = pm; pl > static_cast<char*>(a) && cmp(pl - es, pl) > 0;
                 pl -= es) {
                swapfunc(pl, pl - es, es);
            }
        }
        return;
    }
    pm = static_cast<char*>(a) + (n / 2) * es;
    if (n > 7) {
        pl = static_cast<char*>(a);
        pn = static_cast<char*>(a) + (n - 1) * es;
        if (n > 40) {
            const std::size_t d = (n / 8) * es;
            pl = med3(pl, pl + d, pl + 2 * d, cmp);
            pm = med3(pm - d, pm, pm + d, cmp);
            pn = med3(pn - 2 * d, pn - d, pn, cmp);
        }
        pm = med3(pl, pm, pn, cmp);
    }
    swapfunc(static_cast<char*>(a), pm, es);
    pa = pb = static_cast<char*>(a) + es;
    pc = pd = static_cast<char*>(a) + (n - 1) * es;
    for (;;) {
        while (pb <= pc && (cmp_result = cmp(pb, static_cast<char*>(a))) <= 0) {
            if (cmp_result == 0) {
                swapfunc(pa, pb, es);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (cmp_result = cmp(pc, static_cast<char*>(a))) >= 0) {
            if (cmp_result == 0) {
                swapfunc(pc, pd, es);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc) {
            break;
        }
        swapfunc(pb, pc, es);
        pb += es;
        pc -= es;
    }

    pn = static_cast<char*>(a) + n * es;
    d1 = pa - static_cast<char*>(a);
    if (pb - pa < d1) {
        d1 = static_cast<std::size_t>(pb - pa);
    }
    swapfunc(static_cast<char*>(a), pb - d1, d1);

    d1 = pd - pc;
    d2 = pn - pd - es;
    if (d1 > d2) {
        d1 = d2;
    }
    swapfunc(pb, pn - d1, d1);

    d1 = static_cast<std::size_t>(pb - pa);
    d2 = static_cast<std::size_t>(pd - pc);
    if (d1 <= d2) {
        if (d1 > es) {
            local_qsort(a, d1 / es, es, cmp);
        }
        if (d2 > es) {
            a = pn - d2;
            n = d2 / es;
            goto loop;
        }
    } else {
        if (d2 > es) {
            local_qsort(pn - d2, d2 / es, es, cmp);
        }
        if (d1 > es) {
            n = d1 / es;
            goto loop;
        }
    }
}

} // namespace detail

inline void qsort(void* base, std::size_t nmemb, std::size_t size,
                  QsortCmp compar) noexcept {
    if (nmemb > 1 && size > 0 && compar != nullptr) {
        detail::local_qsort(base, nmemb, size, compar);
    }
}

[[nodiscard]] inline void* bsearch(const void* key, const void* base0,
                                 std::size_t nmemb, std::size_t size,
                                 BsearchCmp compar) noexcept {
    const char* base = static_cast<const char*>(base0);
    std::size_t lim;
    int cmp;
    const void* p;

    for (lim = nmemb; lim != 0; lim >>= 1) {
        p = base + (lim >> 1) * size;
        cmp = compar(key, p);
        if (cmp == 0) {
            return const_cast<void*>(p);
        }
        if (cmp > 0) {
            base = static_cast<const char*>(p) + size;
            --lim;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
