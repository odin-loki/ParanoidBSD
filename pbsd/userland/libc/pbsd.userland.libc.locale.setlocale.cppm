module;

export module pbsd.userland.libc.locale.setlocale;

/// setlocale stub from hbsd/src/lib/libc/locale/setlocale.c
export namespace pbsd::userland::libc::locale {

inline constexpr const char* kDefaultLocale = "C";

enum class Category : unsigned char {
    All, Collate, Ctype, Monetary, Numeric, Time, Messages
};

[[nodiscard]] inline const char* setlocale(Category cat, const char* locale) noexcept {
    (void)cat;
    return locale != nullptr ? locale : kDefaultLocale;
}

} // namespace pbsd::userland::libc::locale
