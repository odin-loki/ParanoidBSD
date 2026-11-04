export module pbsd.kde.frameworks.kcoreaddons.kcrash;

import pbsd.core;

/// Wave 3 — hand port constants (kcrash.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/kcrash.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kcrash {

inline constexpr const char kDrKonqi[] = "drkonqi";
inline constexpr const char kCrashHandlerEnv[] = "KCRASH_HANDLER";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/kcrash.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kcrash
