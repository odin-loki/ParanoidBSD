module;
#include <cstddef>

export module pbsd.userland.libc.stdlib.reallocarray;

export import pbsd.core;

/// reallocarray from hbsd/src/lib/libc/stdlib/reallocarray.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> reallocarray_size(std::size_t nmemb,
                                                           std::size_t size) noexcept {
    if (nmemb != 0 && size > (~std::size_t{0}) / nmemb) {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(nmemb * size);
}

} // namespace pbsd::userland::libc
