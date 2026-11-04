module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbsinit;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int mbsinit(const mbstate_t* ps) noexcept {
    if (ps == nullptr) {
        return 1;
    }
    const auto* p = reinterpret_cast<const unsigned char*>(ps);
    for (std::size_t i = 0; i < sizeof(mbstate_t); ++i) {
        if (p[i] != 0) {
            return 0;
        }
    }
    return 1;
}

} // namespace pbsd::userland::libc::locale
