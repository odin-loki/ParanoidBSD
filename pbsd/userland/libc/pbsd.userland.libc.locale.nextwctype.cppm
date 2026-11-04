module;
#include <cwchar>

export module pbsd.userland.libc.locale.nextwctype;

import pbsd.userland.libc.locale.wctype;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline wctype_t nextwctype(const char* property, wctype_t desc) noexcept {
    (void)desc;
    return wctype(property);
}

} // namespace pbsd::userland::libc::locale
