export module pbsd.kde.eglimagetexture;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (eglimagetexture.cpp).
/// Upstream: kde/kwin/src/opengl/eglimagetexture.cpp
export namespace pbsd::kde::eglimagetexture {

inline constexpr const char kEglImageTarget[] = "GL_OES_EGL_image";
inline constexpr unsigned kMaxTextureSize = 8192;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/opengl/eglimagetexture.cpp";
}

} // namespace pbsd::kde::eglimagetexture
