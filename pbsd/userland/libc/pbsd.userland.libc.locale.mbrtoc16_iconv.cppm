module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbrtoc16_iconv;

import pbsd.userland.libc.locale.mbrtoc16;

/// iconv shim scaffold from hbsd/src/lib/libc/locale/mbrtoc16_iconv.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbrtoc16_iconv(char16_t* pc16, const char* s, std::size_t n,
                                                 mbstate_t* ps) noexcept {
    return pbsd::userland::libc::locale::mbrtoc16(pc16, s, n, ps);
}

} // namespace
