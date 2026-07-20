export module pbsd.kde.frameworks.kcoreaddons.kjob;

import pbsd.core;

/// Wave 3 — KJob error propagation constants.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kjob.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kjob {

inline constexpr int kNoError = 0;
inline constexpr int kUserCanceled = 1;

struct JobState {
    int error{kNoError};
    unsigned percent{0};
    bool finished{false};
};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kjob.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kjob
