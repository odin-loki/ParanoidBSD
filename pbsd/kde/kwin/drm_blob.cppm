export module pbsd.kde.kwin.drm_blob;

import pbsd.core;

/// Wave 3 pass 4 — DRM blob property id stub.
/// Upstream: kde/kwin/src/backends/drm/drm_blob.cpp
export namespace pbsd::kde::kwin::drm_blob {

    inline constexpr unsigned kInvalidBlobId = 0;
    inline constexpr const char kBlobProperty[] = "BLOB_ID";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/backends/drm/drm_blob.cpp";
}

} // namespace pbsd::kde::kwin::drm_blob
