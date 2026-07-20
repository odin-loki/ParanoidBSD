export module pbsd.kde.frameworks.kconfig.kemailsettings;

import pbsd.core;

/// Wave 3 — KEMailSettings profile keys (from kemailsettings.cpp).
/// Upstream: kde/frameworks/kconfig/src/core/kemailsettings.cpp
export namespace pbsd::kde::frameworks::kconfig::kemailsettings {

enum class Setting : unsigned char {
    ClientProgram,
    ClientTerminal,
    RealName,
    EmailAddress,
    ReplyToAddress,
    Organization,
};

inline constexpr const char kProfilePrefix[] = "PROFILE_";
inline constexpr const char kEmailClientKey[] = "EmailClient";
inline constexpr const char kTerminalClientKey[] = "TerminalClient";
inline constexpr const char kFullNameKey[] = "FullName";
inline constexpr const char kEmailAddressKey[] = "EmailAddress";
inline constexpr const char kReplyAddrKey[] = "ReplyAddr";
inline constexpr const char kOrganizationKey[] = "Organization";

[[nodiscard]] inline const char* key_for(Setting s) noexcept {
    switch (s) {
    case Setting::ClientProgram: return kEmailClientKey;
    case Setting::ClientTerminal: return kTerminalClientKey;
    case Setting::RealName: return kFullNameKey;
    case Setting::EmailAddress: return kEmailAddressKey;
    case Setting::ReplyToAddress: return kReplyAddrKey;
    case Setting::Organization: return kOrganizationKey;
    }
    return nullptr;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kemailsettings.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kemailsettings
