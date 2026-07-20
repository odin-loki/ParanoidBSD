export module pbsd.kde.kwin.wayland.idle_inhibit;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 17 — zwp_idle_inhibit_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/idleinhibit_v1.cpp
export namespace pbsd::kde::kwin::wayland::idle_inhibit {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_IDLE_INHIBIT",
    "kwin_idle_inhibit",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/idleinhibit_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_idle_inhibit_manager_v1";
inline constexpr const char kInhibitorInterface[] = "zwp_idle_inhibitor_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Create  = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Create);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/idleinhibit_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::idle_inhibit
