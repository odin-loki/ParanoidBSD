export module pbsd.kde.frameworks.kcoreaddons.klocalizedstring;

import pbsd.core;

/// Wave 3 pass 5 — KLocalizedString domain and markup constants.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/klocalizedstring.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::klocalizedstring {

    inline constexpr const char kDefaultDomain[] = "kde";
    inline constexpr const char kPluralSuffix[] = "_plural";
    inline constexpr unsigned kMaxContextLen = 128;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/klocalizedstring.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::klocalizedstring
