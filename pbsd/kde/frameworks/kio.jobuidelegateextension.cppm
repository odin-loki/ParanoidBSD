export module pbsd.kde.frameworks.kio.jobuidelegateextension;

import pbsd.core;

/// Wave 3 pass 4 — Job UI delegate extension IID.
/// Upstream: kde/frameworks/kio/src/core/jobuidelegateextension.cpp
export namespace pbsd::kde::frameworks::kio::jobuidelegateextension {

    inline constexpr const char kInterfaceIid[] = "org.kde.kio.jobuidelegateextension/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/jobuidelegateextension.cpp";
}

} // namespace pbsd::kde::frameworks::kio::jobuidelegateextension
