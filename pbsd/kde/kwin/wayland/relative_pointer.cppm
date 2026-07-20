export module pbsd.kde.kwin.wayland.relative_pointer;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — zwp_relative_pointer_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/relativepointer_v1.cpp
export namespace pbsd::kde::kwin::wayland::relative_pointer {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_RELATIVE_POINTER",
    "kwin_relative_pointer",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/relativepointer_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_relative_pointer_manager_v1";
inline constexpr const char kPointerInterface[] = "zwp_relative_pointer_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetRelativePointer = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetRelativePointer);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/relativepointer_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::relative_pointer
