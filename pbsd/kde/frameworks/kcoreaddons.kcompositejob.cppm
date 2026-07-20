export module pbsd.kde.frameworks.kcoreaddons.kcompositejob;

import pbsd.core;

/// Wave 3 — composite job subjob registry (from kcompositejob.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kcompositejob.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kcompositejob {

inline constexpr unsigned kMaxSubjobs = 32;

struct SubjobRegistry {
    unsigned ids[kMaxSubjobs]{};
    unsigned count{0};
    int first_error{0};
};

[[nodiscard]] inline Status add_subjob(SubjobRegistry& reg, unsigned job_id) noexcept {
    if (reg.count >= kMaxSubjobs) {
        return Status::NoMemory;
    }
    reg.ids[reg.count++] = job_id;
    return Status::Ok;
}

[[nodiscard]] inline Status propagate_error(SubjobRegistry& reg, int error) noexcept {
    if (error != 0 && reg.first_error == 0) {
        reg.first_error = error;
    }
    return reg.first_error != 0 ? Status::Protocol : Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kcompositejob.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kcompositejob
