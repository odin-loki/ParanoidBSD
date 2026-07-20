export module pbsd.kde.kwin.effects.qpa;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 5 — KWin QPA platform integration stub.
/// Upstream: kde/kwin/src/plugins/qpa/main.cpp
export namespace pbsd::kde::kwin::effects::qpa {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_QPA",
    "kwin_qpa",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/qpa/main.cpp",
};

inline constexpr const char kEffectId[] = "qpa";
inline constexpr const char kDisplayName[] = "QPA";
inline constexpr const char kPluginCategory[] = "System";
inline constexpr const char kMetadataFile[] = "metadata.json.stripped";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/qpa/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::qpa
