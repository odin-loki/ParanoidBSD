module;
#include <cwchar>

export module pbsd.userland.libc.locale.wctype;

export namespace pbsd::userland::libc::locale {

using wctype_t = const void*;

[[nodiscard]] inline wctype_t wctype(const char* property) noexcept {
    (void)property;
    static const int kStub = 0;
    return &kStub;
}

} // namespace pbsd::userland::libc::locale
