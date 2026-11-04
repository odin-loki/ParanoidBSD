export module pbsd.kde.kwin.renderjournal;

import pbsd.core;

/// Wave 3 pass 4 — Render journal frame counter.
/// Upstream: kde/kwin/src/core/renderjournal.cpp
export namespace pbsd::kde::kwin::renderjournal {

    inline constexpr unsigned kMaxFrames = 256;
    inline constexpr unsigned kDefaultHistory = 16;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/renderjournal.cpp";
}

} // namespace pbsd::kde::kwin::renderjournal
