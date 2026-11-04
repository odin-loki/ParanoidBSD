module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memset_s;

export import pbsd.core;

/// memset_s from hbsd/src/lib/libc/string/memset_s.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status memset_s_buf(void* s, std::size_t smax, int c, std::size_t n) noexcept {
    if (s == nullptr || smax < n) return Status::Invalid;
    auto* p = static_cast<unsigned char*>(s);
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = 0; i < n; ++i) p[i] = v;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
