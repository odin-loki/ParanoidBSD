export module pbsd.kde.abstract_opengl_context_attribute_builder;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (abstract_opengl_context_attribute_builder.cpp).
/// Upstream: kde/kwin/src/opengl/abstract_opengl_context_attribute_builder.cpp
export namespace pbsd::kde::abstract_opengl_context_attribute_builder {

inline constexpr unsigned kCoreProfile = 0x0001;
inline constexpr unsigned kCompatProfile = 0x0002;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/opengl/abstract_opengl_context_attribute_builder.cpp";
}

} // namespace pbsd::kde::abstract_opengl_context_attribute_builder
