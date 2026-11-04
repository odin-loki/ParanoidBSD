module;
#include <cwchar>

export module pbsd.userland.libc.locale.collcmp;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int collcmp(const wchar_t* a, const wchar_t* b) noexcept {
    if (a == b) {
        return 0;
    }
    if (a == nullptr) {
        return -1;
    }
    if (b == nullptr) {
        return 1;
    }
    while (*a != L'\0' && *a == *b) {
        ++a;
        ++b;
    }
    if (*a < *b) {
        return -1;
    }
    if (*a > *b) {
        return 1;
    }
    return 0;
}

} // namespace pbsd::userland::libc::locale
