export module pbsd.kde.kwin.wayland.viewporter;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 11 — wp_viewporter crop/scale protocol glue.
/// Upstream: kde/kwin/src/wayland/viewporter.cpp
export namespace pbsd::kde::kwin::wayland::viewporter {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_VIEWPORTER",
    "kwin_viewporter",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/viewporter.cpp",
};

inline constexpr const char kInterface[] = "wp_viewporter";
inline constexpr int kVersion = 1;

enum class ViewporterRequest : unsigned char {
    Destroy        = 0,
    GetViewport    = 1,
};

enum class ViewportRequest : unsigned char {
    Destroy  = 0,
    SetSource = 1,
    SetDestination = 2,
};

[[nodiscard]] inline bool is_viewporter_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ViewporterRequest::GetViewport);
}

[[nodiscard]] inline bool is_viewport_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ViewportRequest::SetDestination);
}

[[nodiscard]] inline Status validate_source(double w, double h) noexcept {
    if (w < 0.0 || h < 0.0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_destination(int w, int h) noexcept {
    if (w < 0 || h < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/viewporter.cpp";
}

} // namespace pbsd::kde::kwin::wayland::viewporter
