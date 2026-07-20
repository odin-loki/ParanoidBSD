module;

#include <cstdio>
#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kjsonutils;

import pbsd.core;

/// Wave 3 — translated JSON key resolution (from KJsonUtils, Qt-free).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/kjsonutils.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kjsonutils {

inline constexpr unsigned kMaxKeyLen = 128;
inline constexpr unsigned kMaxLocaleLen = 32;
inline constexpr unsigned kMaxValueLen = 512;

struct KeyBuffer {
    char data[kMaxKeyLen]{};
};

[[nodiscard]] inline Status make_locale_key(const char* base, const char* locale,
                                            KeyBuffer& out) noexcept {
    if (base == nullptr || locale == nullptr) {
        return Status::Invalid;
    }
    const int n = snprintf(out.data, kMaxKeyLen, "%s[%s]", base, locale);
    if (n <= 0 || static_cast<unsigned>(n) >= kMaxKeyLen) {
        return Status::NoMemory;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status language_from_locale(const char* locale, char* lang,
                                                 unsigned lang_cap) noexcept {
    if (locale == nullptr || lang == nullptr || lang_cap == 0) {
        return Status::Invalid;
    }
    const char* underscore = std::strchr(locale, '_');
    const unsigned len = underscore != nullptr
        ? static_cast<unsigned>(underscore - locale)
        : static_cast<unsigned>(std::strlen(locale));
    if (len >= lang_cap) {
        return Status::NoMemory;
    }
    std::strncpy(lang, locale, len);
    lang[len] = '\0';
    return Status::Ok;
}

enum class LookupOrder : unsigned {
    LocaleWithCountry = 0,
    LanguageOnly = 1,
    BaseKey = 2,
};

[[nodiscard]] inline LookupOrder next_lookup(LookupOrder current) noexcept {
    switch (current) {
    case LookupOrder::LocaleWithCountry:
        return LookupOrder::LanguageOnly;
    case LookupOrder::LanguageOnly:
        return LookupOrder::BaseKey;
    default:
        return LookupOrder::BaseKey;
    }
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/kjsonutils.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kjsonutils
