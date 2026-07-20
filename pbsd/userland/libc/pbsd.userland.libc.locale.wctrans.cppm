module;
#include <cwchar>

export module pbsd.userland.libc.locale.wctrans;

export namespace pbsd::userland::libc::locale {

using wctrans_t = const void*;

[[nodiscard]] inline wctrans_t wctrans(const char* property) noexcept {
    (void)property;
    static const int kStub = 0;
    return &kStub;
}

[[nodiscard]] inline wint_t towctrans(wint_t wc, wctrans_t desc) noexcept {
    (void)desc;
    return wc;
}

} // namespace pbsd::userland::libc::locale
