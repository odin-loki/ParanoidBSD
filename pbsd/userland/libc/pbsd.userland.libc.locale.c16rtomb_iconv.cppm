module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.c16rtomb_iconv;

import pbsd.userland.libc.locale.c16rtomb;

/// iconv shim scaffold from hbsd/src/lib/libc/locale/c16rtomb_iconv.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t c16rtomb_iconv(char* s, char16_t c16, mbstate_t* ps) noexcept {
    return pbsd::userland::libc::locale::c16rtomb(s, c16, ps);
}

} // namespace
