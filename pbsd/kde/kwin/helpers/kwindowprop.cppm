export module pbsd.kde.kwindowprop;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (main.cpp).
/// Upstream: kde/kwin/src/helpers/kwindowprop/main.cpp
export namespace pbsd::kde::kwindowprop {

inline constexpr const char kHelperName[] = "kwindowprop";
inline constexpr unsigned kMaxArgs = 8;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/helpers/kwindowprop/main.cpp";
}

} // namespace pbsd::kde::kwindowprop
