export module pbsd.kde.kwin.effect.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Upstream: kde/kwin/src/effect/logging.cpp
export namespace pbsd::kde::kwin::effect::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "LIBKWINEFFECTS",
    "effect",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/effect/logging.cpp",
};

[[nodiscard]] inline const char* category_name() noexcept { return kCategory.name; }
[[nodiscard]] inline const ::pbsd::kde::kwin::logging::Category& category() noexcept { return kCategory; }

} // namespace pbsd::kde::kwin::effect::logging
