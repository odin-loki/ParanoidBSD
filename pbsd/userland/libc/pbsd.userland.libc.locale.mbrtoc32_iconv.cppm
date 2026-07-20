module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbrtoc32_iconv;

import pbsd.userland.libc.locale.mbrtoc32;

/// iconv shim scaffold from hbsd/src/lib/libc/locale/mbrtoc32_iconv.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbrtoc32_iconv(char32_t* pc32, const char* s, std::size_t n,
                                                 mbstate_t* ps) noexcept {
    return pbsd::userland::libc::locale::mbrtoc32(pc32, s, n, ps);
}

} // namespace
