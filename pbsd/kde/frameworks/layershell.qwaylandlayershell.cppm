export module pbsd.kde.layershell.qwaylandlayershell;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Burst 13 — layer-shell-qt shell manager stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/qwaylandlayershell.cpp
export namespace pbsd::kde::layershell::qwaylandlayershell {

inline constexpr const char kInterface[] = "zwlr_layer_shell_v1";
inline constexpr int kVersion = 4;

enum class Request : unsigned char {
    Destroy = 0,
    GetLayerSurface = 1,
};

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetLayerSurface);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/qwaylandlayershell.cpp";
}

} // namespace pbsd::kde::layershell::qwaylandlayershell
