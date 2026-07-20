export module pbsd.kde.frameworks.kio.kioglobal_unix;

import pbsd.core;

/// Wave 3 — Unix process/symlink helpers (from kioglobal_p_unix.cpp).
/// Upstream: kde/frameworks/kio/src/core/kioglobal_p_unix.cpp
export namespace pbsd::kde::frameworks::kio::kioglobal_unix {

inline constexpr int kSigTerm = 15;

[[nodiscard]] inline bool is_process_alive_result(int kill_result) noexcept {
    return kill_result == 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/kioglobal_p_unix.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kioglobal_unix
