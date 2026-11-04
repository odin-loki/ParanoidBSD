module;
#include <cstdint>

export module pbsd.zfs.vdev;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/vdev_impl.h — vdev states.
export namespace pbsd::zfs::vdev {

enum class State : unsigned char {
    Unknown = 0,
    Closed = 1,
    Offline = 2,
    Removed = 3,
    Faulted = 4,
    Degraded = 5,
    Healthy = 6,
};

enum class Type : unsigned char {
    Unknown = 0,
    Root = 1,
    Mirror = 2,
    Raidz = 3,
    Disk = 4,
    File = 5,
    Hole = 6,
    Spare = 7,
    Log = 8,
    Draid = 9,
};

enum class Aux : unsigned char {
    None = 0,
    External = 1,
    Scratch = 2,
    Log = 3,
    Cache = 4,
    Spare = 5,
};

[[nodiscard]] inline bool is_faulted(State s) noexcept {
    return s == State::Faulted || s == State::Offline;
}

[[nodiscard]] inline bool is_leaf(Type t) noexcept {
    return t == Type::Disk || t == Type::File;
}

[[nodiscard]] inline Status validate_state(State s) noexcept {
    if (static_cast<unsigned char>(s) > static_cast<unsigned char>(State::Healthy)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    if (t == Type::Unknown || t == Type::Hole) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned raidz_parity(Type t, unsigned nparity) noexcept {
    if (t != Type::Raidz) {
        return 0;
    }
    return nparity;
}

struct Label {
    State state{State::Unknown};
    Type type{Type::Unknown};
    unsigned long long size{};
    unsigned ashift{};
};

[[nodiscard]] inline Status validate_label(Label const& lbl) noexcept {
    if (validate_state(lbl.state) != Status::Ok || validate_type(lbl.type) != Status::Ok) {
        return Status::Invalid;
    }
    if (lbl.size == 0 || lbl.ashift < 9 || lbl.ashift > 16) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::vdev
