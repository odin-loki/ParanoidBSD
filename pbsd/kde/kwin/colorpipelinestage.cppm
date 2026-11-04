export module pbsd.kde.kwin.colorpipelinestage;

import pbsd.core;

/// Wave 3 pass 4 — Color pipeline stage tags.
/// Upstream: kde/kwin/src/core/colorpipelinestage.cpp
export namespace pbsd::kde::kwin::colorpipelinestage {

    enum class Stage : unsigned char { Input, Tonemap, Output, Unknown };
    inline constexpr unsigned kMaxStages = 8;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/colorpipelinestage.cpp";
}

} // namespace pbsd::kde::kwin::colorpipelinestage
