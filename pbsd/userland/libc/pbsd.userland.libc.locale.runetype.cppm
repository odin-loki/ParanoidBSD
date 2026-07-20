module;
#include <cstdint>

export module pbsd.userland.libc.locale.runetype;

export namespace pbsd::userland::libc::locale {

using rune_t = std::int32_t;

[[nodiscard]] inline rune_t rune_class(rune_t r) noexcept {
    if (r >= 0 && r < 0x80) {
        return r;
    }
    return 0;
}

} // namespace pbsd::userland::libc::locale
