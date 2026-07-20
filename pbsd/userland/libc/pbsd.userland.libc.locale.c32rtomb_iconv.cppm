module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.c32rtomb_iconv;

import pbsd.userland.libc.locale.c32rtomb;

/// iconv shim scaffold from hbsd/src/lib/libc/locale/c32rtomb_iconv.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t c32rtomb_iconv(char* s, char32_t c32, mbstate_t* ps) noexcept {
    return pbsd::userland::libc::locale::c32rtomb(s, c32, ps);
}

} // namespace
