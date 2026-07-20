export module pbsd.kde.frameworks.kio.emptytrashjob;

import pbsd.core;

/// Wave 3 — hand port constants (emptytrashjob.cpp).
/// Upstream: kde/frameworks/kio/src/core/emptytrashjob.cpp
export namespace pbsd::kde::frameworks::kio::emptytrashjob {

inline constexpr const char kTrashProtocol[] = "trash";
inline constexpr const char kTrashEmptyAction[] = "emptyTrash";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/emptytrashjob.cpp";
}

} // namespace pbsd::kde::frameworks::kio::emptytrashjob
