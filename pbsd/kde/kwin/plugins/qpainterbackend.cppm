export module pbsd.kde.kwin.plugins.qpainterbackend;

import pbsd.core;

/// Wave 3 pass 4 — QPainter compositor backend id.
/// Upstream: kde/kwin/src/qpainter/qpainterbackend.cpp
export namespace pbsd::kde::kwin::plugins::qpainterbackend {

    inline constexpr const char kBackendId[] = "qpainter";
    inline constexpr bool kSupportsEffects = false;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/qpainter/qpainterbackend.cpp";
}

} // namespace pbsd::kde::kwin::plugins::qpainterbackend
