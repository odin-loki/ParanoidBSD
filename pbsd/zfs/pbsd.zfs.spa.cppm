module;

export module pbsd.zfs.spa;

export import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/fs/zfs.h — pool_state_t.
export namespace pbsd::zfs::spa {

enum class State : unsigned char {
    Active = 0,
    Exported = 1,
    Destroyed = 2,
    Spares = 3,
    L2cache = 4,
    Uninitialized = 5,
    Unavailable = 6,
    PotentiallyActive = 7,
};

enum class MmpState : unsigned char {
    Active = 0,
    Inactive = 1,
    NoHostid = 2,
};

[[nodiscard]] constexpr bool is_importable(State s) noexcept {
    return s == State::Exported || s == State::Active || s == State::PotentiallyActive;
}

[[nodiscard]] constexpr bool is_persistent(State s) noexcept {
    return s == State::Active || s == State::Exported || s == State::Spares
        || s == State::L2cache;
}

[[nodiscard]] inline Status validate_state(State s) noexcept {
    const auto v = static_cast<unsigned>(s);
    if (v > static_cast<unsigned>(State::PotentiallyActive)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_mmp(MmpState m) noexcept {
    const auto v = static_cast<unsigned>(m);
    if (v > static_cast<unsigned>(MmpState::NoHostid)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status can_import(State s, MmpState m) noexcept {
    if (validate_state(s) != Status::Ok || validate_mmp(m) != Status::Ok) {
        return Status::Invalid;
    }
    if (!is_importable(s)) {
        return Status::Denied;
    }
    if (m == MmpState::Active) {
        return Status::Busy;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::spa
