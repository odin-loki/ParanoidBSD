module;
#include <cstddef>

export module pbsd.userland.libc.locale.fix_grouping;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline char* fix_grouping(const char* grouping) noexcept {
    return const_cast<char*>(grouping);
}

} // namespace pbsd::userland::libc::locale
