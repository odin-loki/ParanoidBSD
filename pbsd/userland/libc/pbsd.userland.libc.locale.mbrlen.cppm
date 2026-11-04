module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbrlen;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbrlen(const char* s, std::size_t n, mbstate_t* ps) noexcept {
    (void)ps;
    if (s == nullptr) {
        return 0;
    }
    if (n == 0) {
        return static_cast<std::size_t>(-2);
    }
    if (s[0] == '\0') {
        return 0;
    }
    if (static_cast<unsigned char>(s[0]) < 0x80) {
        return 1;
    }
    return static_cast<std::size_t>(-1);
}

} // namespace pbsd::userland::libc::locale
