module;
#include <cstdint>

export module pbsd.zfs.spa_load;

import pbsd.core;
import pbsd.zfs.spa;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/spa.h — pool import/load policy.
export namespace pbsd::zfs::spa_load {

enum class Policy : unsigned char {
    Normal = 0,
    ImportOnly = 1,
    TryImport = 2,
    NoRewind = 3,
};

[[nodiscard]] inline Status validate_policy(Policy p) noexcept {
    switch (p) {
    case Policy::Normal:
    case Policy::ImportOnly:
    case Policy::TryImport:
    case Policy::NoRewind:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status can_load(spa::State state, Policy policy) noexcept {
    if (spa::validate_state(state) != Status::Ok) {
        return Status::Invalid;
    }
    if (policy == Policy::ImportOnly && state == spa::State::Active) {
        return Status::Busy;
    }
    return spa::is_importable(state) ? Status::Ok : Status::Denied;
}

} // namespace pbsd::zfs::spa_load
