export module pbsd.kde.kwin.logging.common;

import pbsd.core;

/// Wave 3 — shared KWin logging category descriptors (Qt-free nucleus view).
export namespace pbsd::kde::kwin::logging {

inline constexpr int kQtDebugMsg = 0;
inline constexpr int kQtWarningMsg = 2;
inline constexpr int kQtCriticalMsg = 3;

struct Category {
    const char* id;
    const char* name;
    int default_level;
    const char* upstream;
};

[[nodiscard]] inline bool category_enabled(const Category& cat, int runtime_level) noexcept {
    return runtime_level >= cat.default_level;
}

} // namespace pbsd::kde::kwin::logging
