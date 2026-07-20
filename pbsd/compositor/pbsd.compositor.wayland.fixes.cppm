module;

export module pbsd.compositor.wayland.fixes;

import pbsd.core;

/// Burst 13 — wl_fixes name/version registry (native compositor).
export namespace pbsd::compositor::wayland::fixes {

inline constexpr const char kInterface[] = "wl_fixes";
inline constexpr int kVersion = 4;

enum class Request : unsigned char {
    Destroy = 0,
    CreateDestroy = 1,
    DestroyRegistry = 2,
    SetGlobalSync = 3,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::SetGlobalSync);
}

[[nodiscard]] inline Status validate_registry_id(unsigned id) noexcept {
    return id > 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::compositor::wayland::fixes
