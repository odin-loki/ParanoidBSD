export module pbsd.kde.kwin.screencast;

import pbsd.core;
import pbsd.kde.kwin.effects.registry;

/// Wave 3 — KWin screencast effect bridge constants.
/// Upstream: kde/kwin/src/plugins/screencast/screencastsource.cpp
export namespace pbsd::kde::kwin::screencast {

inline constexpr const char kEffectId[] = "screencast";

[[nodiscard]] inline bool is_registered() noexcept {
    return kEffectId[0] != '\0';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/screencast/screencastsource.cpp";
}

} // namespace pbsd::kde::kwin::screencast
