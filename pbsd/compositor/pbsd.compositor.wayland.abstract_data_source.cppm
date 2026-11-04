module;

export module pbsd.compositor.wayland.abstract_data_source;

import pbsd.core;

/// Burst 13 — wl_data_source mime limits (native compositor).
export namespace pbsd::compositor::wayland::abstract_data_source {

inline constexpr unsigned kMaxMimeTypes = 16;
inline constexpr unsigned kMaxMimeLen = 128;

enum class SourceRequest : unsigned char {
    Destroy = 0,
    Offer = 1,
    SetActions = 2,
};

[[nodiscard]] inline bool is_source_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SourceRequest::SetActions);
}

[[nodiscard]] inline Status validate_mime_count(unsigned count) noexcept {
    return count <= kMaxMimeTypes ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::compositor::wayland::abstract_data_source
